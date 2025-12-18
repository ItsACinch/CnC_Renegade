# Command & Conquer Renegade - Modernization Plan

## Executive Summary

This document outlines a multi-stage plan to modernize the C&C Renegade codebase from its original Visual C++ 6.0 / Windows XP era implementation to build and run using modern tools on current Windows systems.

## Current State

- **Compiler**: Visual C++ 6.0 (1998)
- **Target OS**: Windows XP (no UAC awareness)
- **Graphics**: DirectX 8.0 (fixed-function pipeline)
- **Audio**: Miles Sound System 6 (proprietary)
- **Video**: RAD Bink (proprietary)
- **Networking**: GameSpy SDK (defunct service)
- **Build System**: Visual Studio .dsp/.dsw project files

## Goals

1. Build with modern MSVC (2019/2022) or compatible compilers
2. Run on Windows 10/11 with proper UAC compliance
3. Replace proprietary SDKs with open-source alternatives
4. Create maintainable CMake-based build system
5. Preserve original gameplay and compatibility where possible

## Modernization Phases

| Phase | Focus Area | Priority | Complexity | Dependencies |
|-------|-----------|----------|------------|--------------|
| 1 | [Build System](01-BUILD-SYSTEM.md) | Critical | Medium | None |
| 2 | [DirectX Modernization](02-DIRECTX.md) | Critical | High | Phase 1 |
| 3 | [Audio System](03-AUDIO.md) | High | Medium | Phase 1 |
| 4 | [Video Playback](04-VIDEO.md) | Medium | Low | Phase 1 |
| 5 | [Networking/Multiplayer](05-NETWORKING.md) | Medium | High | Phase 1 |
| 6 | [Tool SDKs](06-TOOL-SDKS.md) | Low | Medium | Phase 1 |
| 7 | [UAC & Windows Compat](07-UAC-COMPATIBILITY.md) | High | Medium | Phase 1 |

## Dependency Replacement Summary

| Original SDK | Replacement | License | Notes |
|-------------|-------------|---------|-------|
| DirectX 8 | DirectX 11/12 or SDL2+OpenGL | MS/zlib | Major refactor required |
| Miles Sound System | OpenAL Soft + libsndfile | LGPL/LGPL | Good abstraction exists |
| RAD Bink | FFmpeg + custom player | LGPL | Straightforward replacement |
| GameSpy | Custom master server / OpenSpy | - | Service defunct anyway |
| NvDXTLib | libsquish / DirectXTex | MIT/MIT | DXT compression |
| Lightscape | Embree / custom radiosity | Apache | May simplify |
| Umbra | Custom or remove | - | Currently disabled |
| SafeDisk | Remove entirely | - | Obsolete DRM |
| GNU Regex | std::regex (C++11) | - | Built-in replacement |
| MS Cabinet | libmspack or keep FDI | LGPL/MS | Installer only |

## Recommended Approach

### Incremental Migration
Each phase should produce a working build before proceeding to the next. This allows testing and validation at each step.

### Abstraction Layers
The existing codebase has good abstraction layers (WWAudio, BINKMovie wrapper, dx8wrapper). Leverage these for drop-in replacements.

### Compatibility Testing
Maintain a test matrix of original game assets to verify visual/audio fidelity after each change.

## File Structure

```
docs/modernization/
├── 00-OVERVIEW.md          (this file)
├── 01-BUILD-SYSTEM.md      Build system modernization
├── 02-DIRECTX.md           Graphics API migration
├── 03-AUDIO.md             Audio system replacement
├── 04-VIDEO.md             Video playback replacement
├── 05-NETWORKING.md        Network/multiplayer modernization
├── 06-TOOL-SDKS.md         Development tool dependencies
├── 07-UAC-COMPATIBILITY.md Windows compatibility fixes
└── 08-DEPENDENCY-MAP.md    Complete dependency analysis
```

## Success Criteria

1. Clean compilation with MSVC 2022 (no warnings at /W3)
2. Game launches and runs main menu on Windows 11
3. Single-player campaign playable start to finish
4. Multiplayer LAN games functional
5. No administrator privileges required for gameplay
6. All original assets load correctly
