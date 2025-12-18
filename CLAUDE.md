# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

This is the source code for Command & Conquer Renegade, a first-person shooter developed by Westwood Studios. The codebase is Win32 C++ targeting Visual C++ 6.0 (SP5 for binary matching to patch 1.037). Modern MSVC versions require significant code changes due to newer C++ standard enforcement.

## Build Commands

**Main game build (Visual C++ 6.0):**
- Open `Code/commando.dsw` in MSVC 6.0
- Build -> Batch Build -> "Rebuild All"
- Output goes to `/Run/` directory

**Tools build:**
- Open `Code/tools.dsw` for development tools (LevelEdit, W3DView, MixViewer, etc.)

**Special build configurations (defined in `Code/Combat/specialbuilds.h`):**
- Free Dedicated Server: Uncomment `#define FREEDEDICATEDSERVER`, rebuild Release config
- Public Level Editor: Add `PUBLIC_EDITOR_VER` to LevelEdit preprocessor defines
- Beta builds: `BETASERVER`, `BETACLIENT`, `MULTIPLAYERDEMO` (mutually exclusive)

## Architecture

### Core Engine Libraries (Code/ww*)

| Library | Purpose |
|---------|---------|
| `wwlib` | Foundation utilities: memory, strings, file I/O, crypto (Blowfish), containers |
| `wwmath` (WWMath) | Math library: vectors, matrices, quaternions, collision primitives |
| `ww3d2` | 3D rendering engine (W3D format), DirectX 8 wrapper, mesh/animation systems |
| `wwphys` | Physics engine: rigid bodies, collision detection, vehicles, projectiles |
| `wwnet` | Networking: packet management, sockets, NAT traversal |
| `wwsaveload` | Serialization/chunk-based file format system |
| `wwtranslatedb` | Localization/string database |
| `wwui` | UI framework |
| `wwdebug` | Debug/logging utilities |
| `wwbitpack` | Bit packing for network compression |
| `wwutil` | Additional utilities |
| `WWAudio` | Audio system (Miles Sound System integration) |

### Game Layer (Code/Combat, Code/Commando)

- **Combat** - Game logic library: game objects, weapons, vehicles, soldiers, buildings, AI, damage system, scripting interface
- **Commando** - Main executable: WinMain entry point, networking, multiplayer modes (LAN/WOL), dialogs, game modes

Key game object hierarchy: `BaseGameObj` -> `ScriptableGameObj` -> `PhysicalGameObj` -> `DamageableGameObj` -> specific types (SoldierGameObj, VehicleGameObj, BuildingGameObj, etc.)

### Scripts System (Code/Scripts)

DLL-based mission scripting system. Scripts inherit from `ScriptImpClass` and use `ScriptRegistrant` for registration. Mission scripts are in `Mission*.cpp` files. Script commands interface defined in `scriptcommands.h`.

### Supporting Systems

- **BinkMovie** - Video playback (RAD Bink SDK)
- **SControl** - Server control socket interface
- **WWOnline/wolapi/WOLBrowser** - Westwood Online integration
- **Launcher/Installer** - Game launcher and installer

### Tools (Code/Tools)

- **LevelEdit** - Map/level editor
- **W3DView** - W3D model viewer
- **max2w3d** - 3DS Max exporter plugin
- **MixViewer/MakeMix** - Mix archive tools
- **ChunkView** - Chunk file format viewer

## Dependencies (Not Included)

External SDKs expected at specific paths under `Code/`:
- DirectX SDK 8.0+ (`DirectX/`)
- RAD Bink SDK (`BinkMovie/`)
- RAD Miles Sound System (`Miles6/`)
- NvDXTLib, Lightscape, Umbra, GameSpy, SafeDisk, others

## Known Issues

Debug builds of Commando may fail to link due to Windows Defender false positives on RenegadeD.exe. Exclude the `/Run/` folder in Windows Defender.

## RAG Usage Guidelines

When working with this codebase, use the RAG (Retrieval-Augmented Generation) system to maintain knowledge continuity:

**Indexing content:**
- Save useful code snippets, patterns, and solutions discovered during development
- Index documentation files, especially the modernization docs in `docs/modernization/`
- Store examples of common patterns used throughout the codebase (game object creation, serialization, networking, etc.)

**Before code generation:**
- Always query the RAG first to check for existing patterns, snippets, or prior solutions
- This encourages code reuse and consistency with established patterns
- Reduces redundant problem-solving and maintains architectural coherence

**What to index:**
- Reusable utility functions and helper code
- API usage examples for the engine libraries (ww3d2, wwphys, wwnet, etc.)
- Migration patterns for modernization work
- Bug fixes and their solutions
- Build configuration snippets

## Modernization Documentation

A comprehensive multi-phase plan for updating this codebase to build with modern tools is available in `docs/modernization/`:

| Document | Description |
|----------|-------------|
| [00-OVERVIEW.md](docs/modernization/00-OVERVIEW.md) | Executive summary and phase overview |
| [01-BUILD-SYSTEM.md](docs/modernization/01-BUILD-SYSTEM.md) | CMake migration from .dsp/.dsw |
| [02-DIRECTX.md](docs/modernization/02-DIRECTX.md) | DirectX 8 to DirectX 11 migration |
| [03-AUDIO.md](docs/modernization/03-AUDIO.md) | Miles Sound System to OpenAL |
| [04-VIDEO.md](docs/modernization/04-VIDEO.md) | RAD Bink to FFmpeg |
| [05-NETWORKING.md](docs/modernization/05-NETWORKING.md) | GameSpy replacement |
| [06-TOOL-SDKS.md](docs/modernization/06-TOOL-SDKS.md) | Tool SDK replacements |
| [07-UAC-COMPATIBILITY.md](docs/modernization/07-UAC-COMPATIBILITY.md) | Windows 10/11 compatibility |
| [08-DEPENDENCY-MAP.md](docs/modernization/08-DEPENDENCY-MAP.md) | Complete dependency analysis |

Key modernization goals:
- Build with MSVC 2019/2022 and CMake
- Replace proprietary SDKs with open-source alternatives
- Run on Windows 10/11 without administrator privileges
- Estimated effort: 73-108 developer days
