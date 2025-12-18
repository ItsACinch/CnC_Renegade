
# Command & Conquer Renegade

This repository includes source code for Command & Conquer Renegade. This release provides support to the [Steam Workshop](https://steamcommunity.com/workshop/browse/?appid=2229890) for the game.


## Modernization Changes

This fork includes significant modernization work to build and run on modern Windows systems with Visual Studio 2022:

### Build System
- **CMake build system** - Added CMakeLists.txt files for all libraries and the main executable, enabling builds with modern Visual Studio
- **Visual Studio 2022 support** - Project builds successfully with MSVC 2022 (C++17 mode with legacy compatibility flags)

### Graphics (DirectX)
- **DirectX 9 runtime** - Updated from DirectX 8 to DirectX 9, loading d3d9.dll dynamically at runtime
- **Fixed surface copy for MANAGED textures** - Resolved font rendering issues caused by D3D9's stricter texture pool handling
- **Fixed depth buffer z-ordering** - Corrected inverted depth testing that caused rendering artifacts
- **Fixed inline assembly register corruption** - Resolved Color_Convert function crashing due to register clobber issues
- **Screenshot feature** - Added Print Screen key support to save BMP screenshots to the game directory
- **Curved surfaces (N-Patches)** - Updated to use D3D9's `SetNPatchMode()` for software tessellation; option now always available in graphics settings

### Audio (OpenAL)
- **OpenAL Soft replacement for Miles Sound System** - Replaced the proprietary Miles Sound System v6 with OpenAL Soft
- Complete implementation of 2D and 3D audio playback
- WAV file parsing and playback
- Listener positioning for spatial audio
- Timer system for audio callbacks
- **Requires**: Place `OpenAL32.dll` or `soft_oal.dll` in the game's Run directory (download from [OpenAL Soft releases](https://github.com/kcat/openal-soft/releases))

### User Interface
- **Editable graphics options** - Resolution and bit depth can now be changed in the Options menu
- **Settings persistence** - Graphics settings saved to `%TEMP%\Renegade\graphics.ini` and applied on restart
- **In-game restart notification** - Uses proper in-game dialog when settings require restart


## Dependencies

If you wish to rebuild the source code and tools successfully you will need to find or write new replacements (or remove the code using them entirely) for the following libraries:

### Required
- **DirectX SDK** (June 2010) - Expected path `\DirectX-SDK-June2010-master\` or `\Code\DirectX\`
- **OpenAL Soft** - Runtime DLL needed in Run directory ([download](https://github.com/kcat/openal-soft/releases))

### Optional (for full feature support)
- RAD Bink SDK - Video playback (expected path `\Code\BinkMovie\`) - stubbed if missing
- NvDXTLib SDK - (expected path `\Code\NvDXTLib\`)
- Lightscape SDK - (expected path `\Code\Lightscape\`)
- Umbra SDK - (expected path `\Code\Umbra\`)
- GameSpy SDK - (expected path `\Code\GameSpy\`) - stubbed if missing
- GNU Regex - (expected path `\Code\WWLib\`)
- SafeDisk API - (expected path `\Code\Launcher\SafeDisk\`)
- Microsoft Cab Archive Library - (expected path `\Code\Installer\Cab\`)
- RTPatch Library - (expected path `\Code\Installer\`)
- Java Runtime Headers - (expected path `\Code\Tools\RenegadeGR\`)

### No Longer Required
- **Miles Sound System SDK** - Replaced with OpenAL Soft


## Compiling

To use the compiled binaries, you must own the game. The C&C Ultimate Collection is available for purchase on [EA App](https://www.ea.com/en-gb/games/command-and-conquer/command-and-conquer-the-ultimate-collection/buy/pc) or [Steam](https://store.steampowered.com/bundle/39394/Command__Conquer_The_Ultimate_Collection/).

### Modern Build (Visual Studio 2022 + CMake)

1. Install Visual Studio 2022 with C++ workload
2. Install CMake 3.20 or later
3. Place the DirectX SDK (June 2010) at `DirectX-SDK-June2010-master/` in the repository root
4. Generate the build:
   ```cmd
   mkdir build
   cd build
   cmake .. -G "Visual Studio 17 2022" -A Win32
   ```
5. Build:
   ```cmd
   cmake --build . --config Release
   ```
   Or open `build/Renegade.sln` in Visual Studio

6. Place `OpenAL32.dll` (from OpenAL Soft) in the `Run/` directory

### Legacy Build (Visual C++ 6.0)

The quickest way to build all configurations in the project is to open `commando.dsw` in Microsoft Visual Studio C++ 6.0 (SP5 recommended for binary matching to patch 1.037) and select Build -> Batch Build, then hit the "Rebuild All" button.

When the workspace has finished building, the compiled binaries will be copied to the `/Run/` directory found in the root of this repository.


### Free Dedicated Server
It's possible to build the Windows version of the FDS (Free Dedicated Server) for Command & Conquer Renegade from the source code in this repository, just uncomment `#define FREEDEDICATEDSERVER` in [Combat\specialbuilds.h](Combat\specialbuilds.h) and perform a "Rebuild All" action on the Release config.


### Level Edit (Public Release)
To build the public release build of Level Edit, modify the LevelEdit project settings and add `PUBLIC_EDITOR_VER` to the preprocessor defines.


## Running

1. Copy the compiled `Renegade.exe` from `Run/` to your game installation
2. Copy `OpenAL32.dll` (or `soft_oal.dll`) to the game installation directory
3. Run the game


## Known Issues

- The "Debug" configuration may fail to link due to Windows Defender false positives. Exclude the `/Run/` folder in Windows Defender settings.
- Debug builds may crash on startup due to CRT SIMD alignment issues (investigation ongoing)
- Video playback requires the Bink SDK (currently stubbed)


## Contributing

This repository will not be accepting contributions (pull requests, issues, etc). If you wish to create changes to the source code and encourage collaboration, please create a fork of the repository under your GitHub user/organization space.


## Support

This repository is for preservation purposes only and is archived without support.


## License

This repository and its contents are licensed under the GPL v3 license, with additional terms applied. Please see [LICENSE.md](LICENSE.md) for details.
