# Building Renegade (Modernized)

This document describes how to build C&C Renegade using the modernized CMake build system.

## Prerequisites

### Required
- **CMake** 3.20 or later
- **Visual Studio 2019** or **Visual Studio 2022** with C++ workload
- **Windows SDK** 10.0.19041.0 or later

### Optional (for full functionality)
- **DirectX SDK** (June 2010) - for d3dx9 libraries
- Original proprietary SDKs (not redistributable):
  - Miles Sound System 6 - audio
  - RAD Bink SDK - video playback
  - GameSpy SDK - multiplayer (defunct service)

## Quick Start

### Configure
```powershell
# From project root
cmake -B build -G "Visual Studio 17 2022" -A x64
```

### Build
```powershell
# Debug build
cmake --build build --config Debug

# Release build
cmake --build build --config Release

# Profile build (Release with debug info)
cmake --build build --config Profile
```

### Output
Executables are placed in the `Run/` directory:
- `Run/RenegadeD.exe` - Debug build
- `Run/Renegade.exe` - Release build
- `Run/RenegadeP.exe` - Profile build
- `Run/Scripts.dll` - Script system DLL

Libraries are placed in `Code/Libs/<Config>/`:
- `Code/Libs/Debug/` - Debug libraries
- `Code/Libs/Release/` - Release libraries
- `Code/Libs/Profile/` - Profile libraries

## Build Options

Configure with CMake options:

```powershell
# Build dedicated server
cmake -B build -DBUILD_DEDICATED_SERVER=ON

# Build public level editor
cmake -B build -DBUILD_PUBLIC_EDITOR=ON

# Build development tools
cmake -B build -DBUILD_TOOLS=ON

# Build test applications
cmake -B build -DBUILD_TESTS=ON
```

## Troubleshooting

### Missing DirectX Headers
If you get errors about missing `d3dx9.h`:
1. Install the DirectX SDK (June 2010)
2. Or use the DirectX headers from the Windows SDK (limited d3dx9 support)

### Missing Miles/Bink/GameSpy
The build system will automatically stub these if not found:
- `AUDIO_STUB` - Audio functions return silently
- `VIDEO_STUB` - Video playback disabled
- `GAMESPY_STUB` - Multiplayer features disabled

### Compilation Errors
Common issues with modern MSVC:

1. **Exception specifications**: `throw()` is deprecated
   - Fixed by `compiler_compat.h`

2. **`register` keyword**: Removed in C++17
   - Fixed by `compiler_compat.h`

3. **Implicit int**: Old C code may omit return types
   - Will need manual fixes

4. **for loop scoping**: Modern C++ has stricter scoping
   - Will need manual fixes

## Project Structure

```
CnC_Renegade/
├── CMakeLists.txt          # Root CMake configuration
├── cmake/
│   └── CompilerFlags.cmake # Compiler settings
├── Code/
│   ├── compat/
│   │   └── compiler_compat.h  # Compatibility header
│   ├── wwdebug/            # Debug utilities
│   ├── wwlib/              # Foundation library
│   ├── WWMath/             # Math library
│   ├── ww3d2/              # 3D renderer
│   ├── wwphys/             # Physics engine
│   ├── wwnet/              # Networking
│   ├── wwsaveload/         # Serialization
│   ├── wwtranslatedb/      # Localization
│   ├── wwbitpack/          # Network compression
│   ├── wwui/               # UI framework
│   ├── WWAudio/            # Audio system
│   ├── wwutil/             # Utilities
│   ├── BinkMovie/          # Video playback
│   ├── Combat/             # Game logic
│   ├── Scripts/            # Mission scripts (DLL)
│   └── Commando/           # Main executable
├── Run/                    # Output directory
└── docs/
    ├── building.md         # This file
    └── modernization/      # Modernization plans
```

## IDE Setup

### Visual Studio
After running CMake, open `build/Renegade.sln` in Visual Studio.

### Visual Studio Code
1. Install CMake Tools extension
2. Open project folder
3. CMake Tools will auto-detect the CMakeLists.txt
4. Select kit (Visual Studio 2022)
5. Build with F7 or CMake: Build command

## Continuous Integration

GitHub Actions workflow (future):
```yaml
# .github/workflows/build.yml
name: Build
on: [push, pull_request]
jobs:
  build:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v4
      - uses: ilammy/msvc-dev-cmd@v1
      - run: cmake -B build -G "Visual Studio 17 2022" -A x64
      - run: cmake --build build --config Release
```
