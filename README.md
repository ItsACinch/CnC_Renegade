
# Command & Conquer Renegade

This repository includes source code for Command & Conquer Renegade. This release provides support to the [Steam Workshop](https://steamcommunity.com/workshop/browse/?appid=2229890) for the game.


## Dependencies

The following proprietary SDKs have been replaced with open-source alternatives:

- **RAD Bink SDK** - Replaced with FFmpeg (DLLs loaded at runtime)
- **RAD Miles Sound System SDK** - Replaced with OpenAL Soft (DLLs loaded at runtime)

The following proprietary SDKs are no longer required
- Umbra SDK - (expected path `\Code\Umbra\`)
- NvDXTLib SDK - (expected path `\Code\NvDXTLib\`)
- Lightscape SDK - (expected path `\Code\Lightscape\`)
- DirectX SDK (Version 9.0 or higher) (expected path `\Code\DirectX\`)
- GameSpy SDK - (expected path `\Code\GameSpy\`)
- GNU Regex - (expected path `\Code\WWLib\`)
- SafeDisk API - (expected path `\Code\Launcher\SafeDisk\`)

The following libraries are still required for building the installer or optional tools:

- Microsoft Cab Archive Library - (expected path `\Code\Installer\Cab\`)
- RTPatch Library - (expected path `\Code\Installer\`)
- Java Runtime Headers - (expected path `\Code\Tools\RenegadeGR\`)


## Compiling (Win32 Only)

To use the compiled binaries, you must own the game. The C&C Ultimate Collection is available for purchase on [EA App](https://www.ea.com/en-gb/games/command-and-conquer/command-and-conquer-the-ultimate-collection/buy/pc) or [Steam](https://store.steampowered.com/bundle/39394/Command__Conquer_The_Ultimate_Collection/).

### Renegade

The quickest way to build all configurations in the project is to open `commando.dsw` in Microsoft Visual Studio C++ 6.0 (SP5 recommended for binary matching to patch 1.037) and select Build -> Batch Build, then hit the “Rebuild All” button.

If you wish to compile the code under a modern version of Microsoft Visual Studio, you can convert the legacy project file to a modern MSVC solution by opening the `commando.dsw` in Microsoft Visual Studio .NET 2003, and then opening the newly created project and solution file in MSVC 2015 or newer.

NOTE: As modern versions of MSVC enforce newer revisions of the C++ standard, you will need to make extensive changes to the codebase before it successfully compiles, even more so if you plan on compiling for the Win64 platform.

When the workspace has finished building, the compiled binaries will be copied to the `/Run/` directory found in the root of this repository.


### Runtime Requirements

**OpenAL Soft (Audio)**

The game uses OpenAL Soft for audio playback. Download `soft_oal.dll` from [OpenAL Soft releases](https://openal-soft.org/openal-binaries/) and place it in the same directory as `Renegade.exe`. Rename it to `OpenAL32.dll` if needed.

If the OpenAL DLL is not present, the game will run but without audio.

**FFmpeg (Video)**

The game uses FFmpeg for video playback (cutscenes, intro movies). FFmpeg replaces the proprietary RAD Bink Video SDK and can decode Bink video files (`.bik`) as well as many other formats.

Download the shared FFmpeg libraries from one of these sources:
- [Gyan.dev FFmpeg builds](https://www.gyan.dev/ffmpeg/builds/) - Windows builds (recommended: "shared" release build)
- [BtbN FFmpeg builds](https://github.com/BtbN/FFmpeg-Builds/releases) - Cross-platform builds

Extract and place the following DLLs in the same directory as `Renegade.exe`:
- `avcodec-XX.dll` - Video/audio codec library
- `avformat-XX.dll` - Container format handling
- `avutil-XX.dll` - Utility functions
- `swscale-XX.dll` - Video scaling/conversion

Where XX is the FFmpeg major version number (e.g., 60, 61). The game automatically detects versions 55-61.

**Note:** FFmpeg can decode original Bink (`.bik`) video files, so existing game videos will work without conversion. Alternatively, videos can be converted to other formats like MP4/H.264 for potentially better quality.

If the FFmpeg DLLs are not present, the game will run but cutscenes and intro movies will be skipped.


### Free Dedicated Server
It’s possible to build the Windows version of the FDS (Free Dedicated Server) for Command & Conquer Renegade from the source code in this repository, just uncomment `#define FREEDEDICATEDSERVER` in [Combat\specialbuilds.h](Combat\specialbuilds.h) and perform a “Rebuild All” action on the Release config.


### Level Edit (Public Release)
To build the public release build of Level Edit, modify the LevelEdit project settings and add `PUBLIC_EDITOR_VER` to the preprocessor defines.


## Known Issues

The “Debug” configuration of the “Commando” project (the Renegade main project) will sometimes fail to link the final executable. This is due to Windows Defender incorrectly detecting RenegadeD.exe containing a virus (possibly due to the embedded browser code). Excluding the output `/Run/` folder found in the root of this repository in Windows Defender should resolve this for you.


## Contributing

This repository will not be accepting contributions (pull requests, issues, etc). If you wish to create changes to the source code and encourage collaboration, please create a fork of the repository under your GitHub user/organization space.


## Support

This repository is for preservation purposes only and is archived without support. 


## License

This repository and its contents are licensed under the GPL v3 license, with additional terms applied. Please see [LICENSE.md](LICENSE.md) for details.

