# Phase 6: Tool SDK Modernization

## Objective

Replace proprietary SDKs used by development tools with open-source alternatives.

## SDK Dependencies (Tools Only)

| SDK | Used By | Purpose |
|-----|---------|---------|
| NvDXTLib | LevelEdit | DXT texture compression |
| Lightscape | LightMap tool | Radiosity import |
| Umbra | wwphys (disabled) | Occlusion culling |
| SafeDisk | Launcher | Copy protection |
| MS Cabinet (FDI) | Installer | Archive extraction |
| RTPatch | Launcher | Patch application |
| GNU Regex | wwlib | Pattern matching |
| Java JNI | RenegadeGR | Game results reporting |

## Replacement Plan

### 6.1 NvDXTLib → libsquish / DirectXTex

**Current Usage**: `Code/Tools/LevelEdit/TGAToDXT.cpp`
- Compresses TGA to DXT1/DXT5 format
- Called via `nvDXTcompress()`

**Replacement Options**:

#### Option A: libsquish (Recommended for tools)
```cpp
// CMakeLists.txt
find_package(libsquish REQUIRED)

// TGAToDXT.cpp replacement
#include <squish.h>

void CompressToDXT(const uint8_t* rgba, int width, int height,
                   uint8_t* output, bool hasAlpha) {
    int flags = hasAlpha ? squish::kDxt5 : squish::kDxt1;
    flags |= squish::kColourIterativeClusterFit;  // High quality

    squish::CompressImage(rgba, width, height, output, flags);
}
```

#### Option B: DirectXTex (Windows-only, higher quality)
```cpp
#include <DirectXTex.h>

HRESULT CompressToDXT(const Image& srcImage, ScratchImage& dxtImage, bool hasAlpha) {
    DXGI_FORMAT format = hasAlpha ? DXGI_FORMAT_BC3_UNORM : DXGI_FORMAT_BC1_UNORM;
    return Compress(srcImage, format, TEX_COMPRESS_DEFAULT, 1.0f, dxtImage);
}
```

**Recommendation**: libsquish for simplicity, DirectXTex if maximum quality needed.

### 6.2 Lightscape SDK → Simplified/Custom

**Current Usage**: `Code/Tools/LightMap/Lightscape.cpp`
- Imports radiosity solution data from Lightscape .ls files
- Complex builder factory pattern

**Options**:

#### Option A: Remove Lightscape Import
- Pre-bake lightmaps in modern tools (Blender, Unity)
- Export as standard image formats
- Simplify LightMap tool to just pack lightmaps

#### Option B: Custom Radiosity
- Implement simple radiosity using Embree (Intel ray tracing)
- Slower but self-contained

**Recommendation**: Remove Lightscape dependency, use pre-baked lightmaps from modern tools.

```cpp
// Simplified lightmap packer (no Lightscape)
class LightmapPacker {
public:
    // Just pack pre-computed lightmaps into atlas
    bool PackLightmaps(const std::vector<LightmapImage>& inputs,
                       LightmapAtlas& output);
};
```

### 6.3 Umbra SDK → Remove or Custom

**Current Status**: Already disabled (`#define UMBRASUPPORT 0`)

**Options**:

#### Option A: Remove Entirely
- Code is already conditionally compiled out
- Delete umbrasupport.h/cpp files

#### Option B: Simple Frustum Culling
- Already have basic frustum culling
- Good enough for this era of game

#### Option C: Custom Occlusion (If Needed)
```cpp
// Simple software occlusion culling
class OcclusionCuller {
    std::vector<OccluderMesh> occluders;
    DepthBuffer depthBuffer;

public:
    void AddOccluder(const MeshClass& mesh);
    void RenderOccluders(const CameraClass& camera);
    bool IsVisible(const AABoxClass& bounds);
};
```

**Recommendation**: Remove Umbra entirely; not needed for this game's scale.

### 6.4 SafeDisk API → Remove

**Current Usage**: `Code/Launcher/Protect.cpp`
- CD copy protection
- Obsolete technology (causes issues on modern Windows)

**Action**: Remove all SafeDisk code.

```cpp
// Remove these references:
// #include "SafeDisk\CdaPfn.h"
// CDAPFN_DECLARE_GLOBAL(...)
// CDAPFN_ENDMARK(...)

// Replace Protect.cpp with stub:
bool VerifyGameInstallation() {
    // Just check if game files exist
    return FileExists("Data\\always.dat");
}
```

### 6.5 MS Cabinet (FDI) → libmspack or Keep

**Current Usage**: `Code/Installer/CopyThread.cpp`
- Extracts files from .cab archives during installation

**Options**:

#### Option A: Keep FDI (Recommended)
- FDI is part of Windows SDK
- Still works on modern Windows
- No changes needed

#### Option B: libmspack
```cpp
#include <mspack.h>

bool ExtractCabinet(const char* cabPath, const char* destDir) {
    struct mscab_decompressor* decompressor;
    decompressor = mspack_create_cab_decompressor(nullptr);

    struct mscabd_cabinet* cab;
    cab = decompressor->open(decompressor, cabPath);

    for (struct mscabd_file* file = cab->files; file; file = file->next) {
        char destPath[MAX_PATH];
        sprintf(destPath, "%s\\%s", destDir, file->filename);
        decompressor->extract(decompressor, file, destPath);
    }

    decompressor->close(decompressor, cab);
    mspack_destroy_cab_decompressor(decompressor);
    return true;
}
```

**Recommendation**: Keep FDI unless cross-platform needed.

### 6.6 RTPatch → Modern Patching

**Current Usage**: `Code/Launcher/patch.cpp`, `findpatch.cpp`
- Binary delta patching
- Searches for .rtp files in patches/ directory

**Options**:

#### Option A: Remove (Use Game Platform Updates)
- Steam handles updates automatically
- No need for manual patching

#### Option B: Simple File Replacement
```cpp
// Just replace files entirely instead of binary patching
bool ApplyPatch(const char* patchDir) {
    // Copy all files from patch directory to game directory
    CopyDirectory(patchDir, gameDir);
    return true;
}
```

#### Option C: bsdiff/bspatch
```cpp
#include <bspatch.h>

bool ApplyBinaryPatch(const char* oldFile, const char* patchFile, const char* newFile) {
    return bspatch(oldFile, newFile, patchFile) == 0;
}
```

**Recommendation**: Remove patching if distributing via Steam/GOG.

### 6.7 GNU Regex → std::regex

**Current Usage**: `Code/wwlib/regexpr.cpp`
- Pattern matching wrapper class
- Missing gnu_regex.c/h files

**Replacement**: Use C++11 `<regex>`:

```cpp
// Code/wwlib/regexpr.cpp (modernized)
#include <regex>
#include <string>

class RegularExpressionClass {
    std::regex compiledPattern;
    bool valid = false;

public:
    bool Compile(const char* expression) {
        try {
            compiledPattern = std::regex(expression, std::regex::extended);
            valid = true;
        } catch (const std::regex_error&) {
            valid = false;
        }
        return valid;
    }

    bool Match(const char* str) const {
        if (!valid) return false;
        return std::regex_match(str, compiledPattern);
    }

    bool Search(const char* str) const {
        if (!valid) return false;
        return std::regex_search(str, compiledPattern);
    }

    bool Is_Valid() const { return valid; }
};
```

### 6.8 Java JNI → Remove or HTTP API

**Current Usage**: `Code/Tools/RenegadeGR/RenegadeGR.cpp`
- Sends game results to GameRanger backend
- JNI bridge for Java applet

**Options**:

#### Option A: Remove Entirely
- GameRanger integration likely obsolete
- Remove JNI code

#### Option B: Replace with HTTP
```cpp
// Direct HTTP reporting instead of JNI
bool ReportGameResults(const GameResults& results) {
    json body = {
        {"gameId", results.gameId},
        {"players", json::array()}
    };

    for (const auto& player : results.players) {
        body["players"].push_back({
            {"name", player.name},
            {"score", player.score}
        });
    }

    return HttpClient::Post("https://stats.example.com/api/results", body.dump());
}
```

**Recommendation**: Remove unless specific stats service integration needed.

## Summary of Actions

| SDK | Action | Replacement |
|-----|--------|-------------|
| NvDXTLib | Replace | libsquish |
| Lightscape | Remove | Pre-baked lightmaps |
| Umbra | Remove | (already disabled) |
| SafeDisk | Remove | Simple file check |
| MS Cabinet | Keep | (Windows SDK) |
| RTPatch | Remove | Platform updates |
| GNU Regex | Replace | std::regex |
| Java JNI | Remove | HTTP API if needed |

## CMake Integration

```cmake
# Tool dependencies
if(BUILD_TOOLS)
    # libsquish for texture compression
    find_package(libsquish CONFIG REQUIRED)

    # LevelEdit
    add_executable(LevelEdit ${LEVELEDIT_SOURCES})
    target_link_libraries(LevelEdit PRIVATE
        libsquish::squish
        ww3d2
        wwlib
    )

    # LightMap (simplified)
    add_executable(LightMap ${LIGHTMAP_SOURCES})
    target_link_libraries(LightMap PRIVATE
        ww3d2
        wwlib
    )
endif()
```

## Verification

1. LevelEdit can compress textures to DXT
2. LightMap tool functions (if kept)
3. Game launches without SafeDisk
4. Regex operations work correctly
5. All tools compile and run

## Estimated Effort

- NvDXTLib replacement: 1-2 days
- Lightscape removal: 1 day
- Umbra removal: 0.5 day
- SafeDisk removal: 0.5 day
- GNU Regex replacement: 1 day
- JNI removal: 0.5 day
- Testing: 2-3 days
- **Total: 6-9 days**

## Risks

1. **Texture quality differences**: libsquish may produce slightly different results
2. **Lightmap workflow**: Artists need new workflow for lightmaps
3. **Regex behavior**: std::regex has some differences from GNU regex
