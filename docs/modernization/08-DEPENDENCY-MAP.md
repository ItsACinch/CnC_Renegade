# Dependency Analysis and Mapping

## Complete Dependency Inventory

### External SDK Dependencies

| SDK | Version | License | Files Using It | Replacement |
|-----|---------|---------|----------------|-------------|
| DirectX | 8.0 | Microsoft | 19 dx8*.cpp files, directinput.cpp | DirectX 11 (MS SDK) |
| Miles Sound System | 6.x | Proprietary | WWAudio/*.cpp (15+ files) | OpenAL Soft + libsndfile |
| RAD Bink | 1.x | Proprietary | BinkMovie/*.cpp (6 files) | FFmpeg |
| GameSpy SDK | - | Proprietary | 8 gamespy*.cpp files | Custom/Remove |
| NvDXTLib | - | NVIDIA | TGAToDXT.cpp | libsquish / DirectXTex |
| Lightscape SDK | - | Proprietary | Lightscape.cpp | Remove/simplify |
| Umbra SDK | - | Proprietary | umbrasupport.cpp | Remove (disabled) |
| SafeDisk | - | Proprietary | Protect.cpp | Remove |
| GNU Regex | - | GPL | regexpr.cpp | std::regex |
| Java JNI | 1.x | Oracle | RenegadeGR.cpp | Remove/HTTP |

### Windows SDK Dependencies

| Component | Usage | Status |
|-----------|-------|--------|
| WinSock 2 | Networking | Keep |
| WinHTTP | HTTP requests | Add (new) |
| FDI (Cabinet) | Archive extraction | Keep |
| DirectShow | (unused) | - |
| GDI/GDI+ | 2D graphics | Keep |
| Shell API | File dialogs, paths | Keep |

### C Runtime Dependencies

| Library | Usage | Notes |
|---------|-------|-------|
| MSVCRT | Standard C library | Update to UCRT |
| MFC | UI dialogs (tools) | Keep for tools |

## Dependency Graph

```
┌─────────────────────────────────────────────────────────────────┐
│                        RENEGADE.EXE                              │
└─────────────────────────────────────────────────────────────────┘
    │
    ├── Combat.lib
    │     ├── ww3d2.lib ──────────► DirectX 8 → DirectX 11
    │     ├── wwphys.lib ─────────► (Umbra - disabled)
    │     ├── wwsaveload.lib
    │     └── wwtranslatedb.lib
    │
    ├── WWAudio.lib ──────────────► Miles 6 → OpenAL Soft
    │
    ├── BinkMovie.lib ────────────► RAD Bink → FFmpeg
    │
    ├── wwnet.lib ────────────────► WinSock 2 (keep)
    │     └── GameSpy SDK ────────► Custom/Remove
    │
    ├── wwlib.lib
    │     ├── GNU Regex ──────────► std::regex
    │     └── DirectDraw ─────────► Remove (legacy)
    │
    ├── Scripts.dll
    │
    └── SControl.lib
          └── WinSock 2 (keep)

┌─────────────────────────────────────────────────────────────────┐
│                         TOOLS                                    │
└─────────────────────────────────────────────────────────────────┘
    │
    ├── LevelEdit.exe
    │     ├── ww3d2.lib
    │     └── NvDXTLib ───────────► libsquish
    │
    ├── LightMap.exe
    │     └── Lightscape SDK ─────► Remove/simplify
    │
    ├── W3DView.exe
    │     └── ww3d2.lib
    │
    └── max2w3d.dll (3DS Max plugin)
          └── Max SDK (external)

┌─────────────────────────────────────────────────────────────────┐
│                        LAUNCHER                                  │
└─────────────────────────────────────────────────────────────────┘
    │
    ├── SafeDisk ─────────────────► Remove
    └── RTPatch ──────────────────► Remove

┌─────────────────────────────────────────────────────────────────┐
│                        INSTALLER                                 │
└─────────────────────────────────────────────────────────────────┘
    │
    └── MS Cabinet (FDI) ─────────► Keep (Windows SDK)
```

## Header File Dependencies

### DirectX Headers (to replace)
```
d3d8.h          → d3d11.h
d3dx8core.h     → (removed, use DirectXMath)
d3dx8math.h     → DirectXMath.h
dinput.h        → (Raw Input or keep for compatibility)
dsound.h        → (via audio middleware)
ddraw.h         → (remove)
```

### Miles Headers (to replace)
```
mss.h           → al.h, alc.h (OpenAL)
```

### Bink Headers (to replace)
```
Bink.h          → libavcodec/avcodec.h, etc.
RAD.h           → (remove)
```

### GameSpy Headers (to remove)
```
gqueryreporting.h → (custom implementation)
gcdkeyserver.h    → (remove)
gcdkeyclient.h    → (remove)
ghttp.h           → winhttp.h
gs_md5.h          → (remove or use OpenSSL)
```

## Library Linking Changes

### Current Link Libraries (Debug)
```
d3dx8.lib
dinput.lib
dxguid.lib
dsound.lib
mss32.lib
binkw32.lib
wsock32.lib
```

### New Link Libraries (Debug)
```
d3d11.lib
dxgi.lib
d3dcompiler.lib
OpenAL32.lib
sndfile.lib
avcodec.lib
avformat.lib
avutil.lib
swscale.lib
ws2_32.lib
winhttp.lib
```

## File-by-File Dependency Analysis

### High Priority (Blocks everything)
| File | Dependencies | Priority |
|------|-------------|----------|
| dx8wrapper.cpp | d3d8.h, d3dx8core.h | Critical |
| dx8caps.cpp | d3d8.h | Critical |
| WWAudio.cpp | mss.h | Critical |
| BINKMovie.cpp | Bink.h | High |

### Medium Priority (Networking)
| File | Dependencies | Priority |
|------|-------------|----------|
| GameSpy_QnR.cpp | gqueryreporting.h | Medium |
| gamespyauthmgr.cpp | gcdkeyserver.h | Medium |
| CDKeyAuth.cpp | gcdkeyclient.h | Medium |

### Low Priority (Tools only)
| File | Dependencies | Priority |
|------|-------------|----------|
| TGAToDXT.cpp | NvDXTLib.h | Low |
| Lightscape.cpp | Lightscape SDK | Low |
| umbrasupport.cpp | Umbra SDK | Low (disabled) |
| Protect.cpp | SafeDisk | Low |

## Conditional Compilation Flags

### Existing Flags
```cpp
#define FREEDEDICATEDSERVER    // Dedicated server build
#define BETASERVER             // Beta server
#define BETACLIENT             // Beta client
#define MULTIPLAYERDEMO        // Demo build
#define PUBLIC_EDITOR_VER      // Public LevelEdit
#define UMBRASUPPORT 0         // Umbra disabled
```

### Proposed New Flags
```cpp
#define USE_DIRECTX11          // Use D3D11 instead of D3D8
#define USE_OPENAL             // Use OpenAL instead of Miles
#define USE_FFMPEG             // Use FFmpeg instead of Bink
#define USE_MODERN_NETWORKING  // Use custom master server
#define UAC_COMPLIANT          // User-mode only operation
```

## Migration Order

Based on dependencies, the recommended migration order is:

```
Phase 1: Build System (CMake)
    │
    └─► Phase 2: DirectX (Critical path)
          │
          ├─► Phase 3: Audio (Parallel possible)
          │
          ├─► Phase 4: Video (Parallel possible)
          │
          └─► Phase 7: UAC (Can start early)

Phase 5: Networking (After basic game works)
    │
    └─► Phase 6: Tool SDKs (Last, lowest priority)
```

## Estimated Total Effort

| Phase | Days | Cumulative |
|-------|------|------------|
| 1. Build System | 7-11 | 7-11 |
| 2. DirectX | 25-36 | 32-47 |
| 3. Audio | 11-16 | 43-63 |
| 4. Video | 7-11 | 50-74 |
| 5. Networking | 10-15 | 60-89 |
| 6. Tool SDKs | 6-9 | 66-98 |
| 7. UAC | 7-10 | 73-108 |

**Total Estimated Effort: 73-108 developer days**

With parallel work on phases 3, 4, and 7 after phase 2 begins:
**Optimistic Timeline: 50-60 days with 2 developers**

## Risk Matrix

| Risk | Impact | Probability | Mitigation |
|------|--------|-------------|------------|
| D3D8→D3D11 visual differences | High | Medium | Extensive comparison testing |
| Audio sync issues | Medium | Medium | Careful buffer management |
| Network protocol changes | High | Low | Version negotiation |
| Save game incompatibility | Medium | Low | Migration tool |
| Antivirus false positives | Low | Medium | Code signing |
| Tool workflow disruption | Medium | Medium | Documentation |
