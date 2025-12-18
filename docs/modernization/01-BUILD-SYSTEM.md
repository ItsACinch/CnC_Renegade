# Phase 1: Build System Modernization

## Objective

Convert from Visual C++ 6.0 .dsp/.dsw project files to a modern CMake-based build system that works with MSVC 2019/2022.

## Current Build Structure

### Workspace Files
- `Code/commando.dsw` - Main game workspace (21 projects)
- `Code/tools.dsw` - Development tools workspace (25 projects)

### Project Dependencies (commando.dsp)
```
commando.exe
├── Combat.lib
├── Scripts.dll
├── BinkMovie.lib
├── BandTest.lib
├── SControl.lib
├── GameSpy.lib
├── ww3d2.lib
├── wwphys.lib
├── wwnet.lib
├── wwsaveload.lib
├── wwtranslatedb.lib
├── wwbitpack.lib
├── wwui.lib
├── WWAudio.lib
├── wwutil.lib
├── wwmath.lib
├── wwlib.lib
└── wwdebug.lib
```

## Tasks

### 1.1 Create Root CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.20)
project(Renegade VERSION 1.037 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Run)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Run)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Code/Libs/${CMAKE_BUILD_TYPE})

# Global definitions matching original build
add_compile_definitions(
    WIN32
    _WINDOWS
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

# Subdirectories
add_subdirectory(Code/wwdebug)
add_subdirectory(Code/wwlib)
add_subdirectory(Code/WWMath)
add_subdirectory(Code/ww3d2)
add_subdirectory(Code/wwphys)
add_subdirectory(Code/wwnet)
add_subdirectory(Code/wwsaveload)
add_subdirectory(Code/wwtranslatedb)
add_subdirectory(Code/wwbitpack)
add_subdirectory(Code/wwui)
add_subdirectory(Code/WWAudio)
add_subdirectory(Code/wwutil)
add_subdirectory(Code/BinkMovie)
add_subdirectory(Code/Combat)
add_subdirectory(Code/Scripts)
add_subdirectory(Code/Commando)
```

### 1.2 Create Per-Library CMakeLists.txt

Each library needs its own CMakeLists.txt. Example for wwlib:

```cmake
# Code/wwlib/CMakeLists.txt
file(GLOB WWLIB_SOURCES "*.cpp" "*.c")
file(GLOB WWLIB_HEADERS "*.h")

add_library(wwlib STATIC ${WWLIB_SOURCES} ${WWLIB_HEADERS})

target_include_directories(wwlib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/Code
)

target_link_libraries(wwlib PUBLIC wwdebug)
```

### 1.3 Handle C++ Standard Incompatibilities

The original code uses pre-C++98 patterns. Key issues to address:

| Issue | Files Affected | Fix |
|-------|---------------|-----|
| `for` loop variable scope | Many | Already scoped in modern C++ |
| Exception specifications | Many headers | Remove `throw()` specs |
| `auto_ptr` usage | wwlib | Replace with `unique_ptr` |
| Implicit `int` | Old C code | Add explicit types |
| K&R function declarations | Some C files | Convert to ANSI |
| `register` keyword | Performance code | Remove (ignored in C++17) |
| Trigraphs | String literals | Disable or replace |

### 1.4 Create Compiler Compatibility Header

```cpp
// Code/compat/compiler_compat.h
#pragma once

// Disable deprecated warnings for legacy code
#pragma warning(disable: 4996) // deprecated functions
#pragma warning(disable: 4244) // conversion warnings
#pragma warning(disable: 4267) // size_t to int

// Handle removed keywords
#define register

// Handle exception specs
#define THROW_SPEC(x)

// Platform detection
#if defined(_MSC_VER) && _MSC_VER >= 1900
    #define MODERN_MSVC 1
#endif
```

### 1.5 DirectX SDK Setup

Modern DirectX is part of Windows SDK. Create find module:

```cmake
# cmake/FindDirectX.cmake
find_path(DIRECTX_INCLUDE_DIR d3d11.h
    PATHS "$ENV{WindowsSdkDir}/Include/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}/um"
)

find_library(D3D11_LIBRARY d3d11
    PATHS "$ENV{WindowsSdkDir}/Lib/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}/um/x64"
)
```

### 1.6 External Dependencies CMake Config

```cmake
# cmake/ExternalDeps.cmake

# OpenAL Soft (Miles replacement)
find_package(OpenAL CONFIG REQUIRED)

# FFmpeg (Bink replacement)
find_package(FFmpeg REQUIRED COMPONENTS avcodec avformat avutil swscale)

# SDL2 (optional, for cross-platform input)
find_package(SDL2 CONFIG)
```

### 1.7 Build Configurations

```cmake
# Support original build types
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;Profile" CACHE STRING "" FORCE)

# Profile = Release with debug info
set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
set(CMAKE_EXE_LINKER_FLAGS_PROFILE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG")
```

## Deliverables

1. `CMakeLists.txt` files for all 21+ projects
2. `cmake/` folder with find modules and toolchain files
3. `Code/compat/` folder with compatibility headers
4. Build documentation in `docs/building.md`
5. CI/CD configuration (GitHub Actions)

## Verification

```powershell
# Configure
cmake -B build -G "Visual Studio 17 2022" -A x64

# Build Debug
cmake --build build --config Debug

# Build Release
cmake --build build --config Release

# Verify output
Test-Path Run/Renegade.exe
Test-Path Run/Scripts.dll
```

## Estimated Effort

- CMakeLists.txt creation: 2-3 days
- Compiler compatibility fixes: 3-5 days
- Testing and debugging: 2-3 days
- **Total: 7-11 days**

## Risks

1. **Preprocessor differences**: Modern MSVC may expand macros differently
2. **Struct packing**: `#pragma pack` behavior may differ
3. **Inline assembly**: Any x86 ASM needs review (rare in this codebase)
4. **Windows SDK version**: Some legacy APIs may be removed
