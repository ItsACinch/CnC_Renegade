# Modernization Progress Log

## Overview

This document tracks the progress of the C&C Renegade modernization effort.

---

## Phase 1 & 2: Build System and Initial Compilation (COMPLETED)

**Status**: Core game executable builds successfully with MSVC 2022

**Date Completed**: December 2024

### Accomplishments

1. **CMake Build System Created**
   - Root CMakeLists.txt with all library targets
   - Individual CMakeLists.txt for each library
   - Win32 (x86) target configuration
   - Debug/Release/Profile configurations
   - Output to original directory structure (Code/Libs/, Run/)

2. **DirectX 8 to DirectX 9 Compatibility Layer**
   - Created `Code/DirectX/include/d3d8_compat.h` - D3D8 to D3D9 type mappings
   - Updated `dx8wrapper.h/cpp` for D3D9 API compatibility
   - Created stub implementations for removed D3D8-specific APIs

3. **Miles Sound System Stubs**
   - Created `Code/Miles6/include/mss.h` with function stubs
   - All audio functions return success but do nothing
   - Allows compilation without proprietary Miles SDK

4. **Bink Video Stubs**
   - Created `Code/BinkMovie/bink_stub.h` with Bink API stubs
   - Video playback disabled but code compiles

5. **GameSpy SDK Stubs**
   - `gqueryreporting.h` - Query reporting functions
   - `gcdkeyserver.h` - CD key server authentication
   - `gcdkeyclient.h` - CD key client
   - `gs_md5.h` - MD5 digest stub
   - `gs_patch_usage.h` - Patch usage tracking
   - `ghttp.h` - HTTP request stubs
   - `nonport.h` - Portable type definitions

### C++ Modernization Fixes

#### Member Function Pointer Syntax (Modern C++ Compliance)
- **Files**: `statemachine.h`, `raveshawbossgameobj.h/cpp`, `mendozabossgameobj.h/cpp`
- **Issue**: MSVC6 allowed `On_Method` for member function pointers, modern C++ requires `&ClassName::On_Method`
- **Fix**: Added `typedef ThisClass` and updated macros/code to use `&ThisClass::Method`

#### Typename Keyword for Dependent Types
- **Files**: `statemachine.h`
- **Issue**: Template dependent types need `typename` keyword
- **Fix**: Added `typename` to function parameter types

#### Private Member Access via Friend Classes
- **Files**: `slist.h`
- **Issue**: Friend class access doesn't extend to base class private members
- **Fix**: Changed direct `NodeNext` access to use `Set_Next()` accessor

#### Const Correctness for String Functions
- **Files**: `mapmgr.cpp`, `assetdep.cpp`, `building.cpp`, `soldier.cpp`, `savegame.cpp`, `WOLLogonMgr.cpp`, `WOLSession.cpp`, `DlgMPTeamSelect.cpp`, `WOLQuickMatch.cpp`, `consolefunction.cpp`
- **Issue**: `strchr`, `strstr`, `wcsstr`, `wcschr`, `wcsrchr` return `const char*`/`const wchar_t*` in modern C++
- **Fix**: Changed variable types to `const` or used `const_cast` where modification is needed

#### Missing Type Specifiers
- **Files**: `consolemode.h`, `ConsoleMode.cpp`, `dlgcncbattleinfo.cpp`
- **Issue**: Missing return types and `int` type in const declarations
- **Fix**: Added explicit types

#### Method Call Syntax
- **Files**: `hud.cpp`, `scriptzone.cpp`, `assetdep.cpp`
- **Issue**: `.Count` instead of `.Count()`
- **Fix**: Added parentheses

#### Missing Includes
- **Files**: `viseme.cpp`, `WebBrowser.cpp`, `GameSpy_QnR.cpp`
- **Issue**: Missing `<ctype.h>`, `<shellapi.h>`, `<time.h>`
- **Fix**: Added includes

#### Variable Name Collisions
- **Files**: `weaponbag.cpp`
- **Issue**: Loop variable `weapon` shadowed class member
- **Fix**: Renamed loop variable to `weapon_idx`

#### DEFINE_AUTO_POOL Macro Access
- **Files**: `weathermgr.h`
- **Issue**: Structs used with macro need public access
- **Fix**: Moved structs to public section

#### SEH vs C++ Exception Handling
- **Files**: `WINMAIN.CPP`
- **Issue**: `__try/__except` can't be used with C++ objects requiring stack unwinding
- **Fix**: Replaced with `SetUnhandledExceptionFilter`

#### Iterator to Pointer Comparison
- **Files**: `WOLNetUtilObserver.cpp`
- **Issue**: Comparing `std::vector<T>::iterator` with `T*`
- **Fix**: Changed to `&(*iter) == pointer`

#### RefPtr Template Conversion
- **Files**: `RefPtr.h`
- **Issue**: Template conversion operator returning wrong type
- **Fix**: Added `static_cast<NewType*>` to GetRefObject() result

#### Resource File MFC Dependency
- **Files**: `chat.rc`
- **Issue**: `afxres.h` requires MFC
- **Fix**: Replaced with `<windows.h>` and manual `IDC_STATIC` definition

#### Missing Resource IDs
- **Files**: `dialogresource.h`
- **Issue**: `IDC_MP_SHORTCUT_INSIDER` undefined
- **Fix**: Added definition

#### Preprocessor Guards
- **Files**: `dlgmpwolquickmatch.cpp`
- **Issue**: Code using `QUICKMATCH_OPTIONS` guarded class
- **Fix**: Added matching preprocessor guards

#### Static Library DLL Import/Export
- **Files**: `BandTest.h`, `BandTest/CMakeLists.txt`
- **Issue**: `BANDTEST_API` set to `dllimport` for static lib users
- **Fix**: Added `BANDTEST_STATIC` option for static library builds

### Libraries Built Successfully

All 17 core libraries now compile:
1. wwdebug
2. wwlib
3. wwnet
4. wwmath
5. ww3d2
6. wwphys
7. wwsaveload
8. wwtranslatedb
9. wwbitpack
10. wwui
11. wwaudio
12. wwutil
13. combat
14. binkmovie
15. bandtest
16. scontrol
17. gamespy

### Main Executable

- **RenegadeD.exe** (Debug) builds successfully (~13.4 MB)
- Output location: `Run/RenegadeD.exe`

### Build Command

```powershell
# Configure for Win32 (x86)
cmake -B build -G "Visual Studio 17 2022" -A Win32

# Build Debug
cmake --build build --config Debug --target renegade

# Build all
cmake --build build --config Debug
```

---

## Phase 7: UAC Compatibility (IN PROGRESS)

**Status**: Registry access fixed, application manifest added

**Date Started**: December 2024

### Accomplishments

1. **Registry Migration to HKEY_CURRENT_USER**
   - **File**: `Code/wwlib/registry.cpp`
   - **Issue**: Original code used `HKEY_LOCAL_MACHINE` which requires admin privileges on Windows Vista+
   - **Fix**: Changed all registry access to use `HKEY_CURRENT_USER`
   - **Functions Modified**:
     - `RegistryClass::Exists()` - Changed HKLM to HKCU
     - `RegistryClass::RegistryClass()` constructor - Changed HKLM to HKCU for both create and open
     - `RegistryClass::Save_Registry_Tree()` - Changed HKLM to HKCU
     - `RegistryClass::Delete_Registry_Tree()` - Changed HKLM to HKCU

2. **Application Manifest for UAC**
   - **File Created**: `manifests/Renegade.exe.manifest`
   - **Features**:
     - `requestedExecutionLevel="asInvoker"` - No admin elevation required
     - Windows 10/11 compatibility declarations
     - DPI awareness settings (PerMonitorV2)
   - **CMake Integration**: `Code/Commando/CMakeLists.txt` updated to embed manifest

### Registry Path
Settings are now stored at: `HKEY_CURRENT_USER\Software\Westwood\Renegade\...`

Instead of: `HKEY_LOCAL_MACHINE\Software\Westwood\Renegade\...` (requires admin)

---

## Remaining Work

### Phase 2 Completion: DirectX Modernization
- [ ] Test rendering with actual game assets
- [ ] Fix any runtime D3D9 issues
- [ ] Implement proper shader pipeline (currently fixed-function)

### Phase 3: Audio System
- [ ] Replace Miles stubs with OpenAL Soft implementation
- [ ] Add audio file format support (libsndfile)

### Phase 4: Video Playback
- [ ] Replace Bink stubs with FFmpeg implementation
- [ ] Test video playback in game

### Phase 5: Networking
- [ ] Implement replacement master server protocol
- [ ] Test LAN multiplayer

### Phase 6: Tool SDKs
- [ ] Build LevelEdit and other tools
- [ ] Replace tool-specific proprietary SDKs

### Phase 7: Windows Compatibility
- [x] Fix UAC/path issues (registry moved to HKCU)
- [x] Update registry usage
- [x] Add application manifest
- [ ] Test on Windows 10/11
- [ ] Implement user data directory migration (saves, configs to %LOCALAPPDATA%)
- [ ] Handle VirtualStore data migration

---

## Known Issues

1. **Scripts DLL not building** - Mission scripts excluded due to default argument issues
2. **Audio disabled** - Miles SDK stubs only
3. **Video disabled** - Bink SDK stubs only
4. **GameSpy networking disabled** - Stub implementation only
5. **Runtime untested** - Executable builds but not tested with game data

---

## File Reference

### Created Files
- `CMakeLists.txt` (root)
- `Code/*/CMakeLists.txt` (per-library)
- `Code/DirectX/include/d3d8_compat.h`
- `Code/Miles6/include/mss.h`
- `Code/BinkMovie/bink_stub.h`
- `Code/GameSpy/gqueryreporting.h`
- `Code/GameSpy/gcdkeyserver.h`
- `Code/GameSpy/gcdkeyclient.h`
- `Code/GameSpy/gs_md5.h`
- `Code/GameSpy/gs_patch_usage.h`
- `Code/GameSpy/ghttp.h`
- `Code/GameSpy/nonport.h`
- `manifests/Renegade.exe.manifest` - UAC manifest for Windows 10/11 compatibility

### Modified Files (Key Changes)
- `Code/Combat/statemachine.h` - typename keyword, macro update
- `Code/Combat/raveshawbossgameobj.h` - ThisClass typedef
- `Code/Combat/mendozabossgameobj.h` - ThisClass typedef
- `Code/Combat/weathermgr.h` - public struct access
- `Code/Scripts/slist.h` - Set_Next() accessor usage
- `Code/BandTest/BandTest.h` - BANDTEST_STATIC support
- `Code/Commando/WINMAIN.CPP` - SEH removal
- `Code/Commando/chat.rc` - MFC removal
- `Code/Commando/CMakeLists.txt` - Manifest embedding
- `Code/WWOnline/RefPtr.h` - Template cast fix
- `Code/wwlib/registry.cpp` - HKEY_LOCAL_MACHINE to HKEY_CURRENT_USER migration
- Various files: const correctness, missing includes
