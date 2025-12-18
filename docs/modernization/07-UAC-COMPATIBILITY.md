# Phase 7: UAC and Windows Compatibility

## Objective

Ensure the game runs on Windows 10/11 without requiring administrator privileges and complies with modern Windows security practices.

## Current Issues Identified

### Registry Access Problems

| Location | Issue | Severity |
|----------|-------|----------|
| `HKEY_LOCAL_MACHINE\Software\Westwood\Renegade` | Writes require admin | High |
| `HKLM\...\CurrentVersion\Uninstall` | Installer registry | Medium |
| `HKLM\...\CurrentVersion\App Paths` | Application registration | Low |
| `HKLM\...\CurrentVersion\RunOnce` | Auto-start entries | Medium |

### File System Problems

| Path | Issue | Severity |
|------|-------|----------|
| `Program Files\` installation | Writes require admin | High |
| Game save/config in install dir | VirtualStore redirection | High |
| DirectX/redistributables | System directory access | Medium |

### Privilege Escalation Code

| File | Function | Issue |
|------|----------|-------|
| `Launcher/patch.cpp` | `Shutdown_Computer_Now()` | Acquires SE_SHUTDOWN_NAME |
| `Installer/WinMain.cpp` | `Running_As_Administrator()` | Requires admin to run |

### Missing Manifest

No UAC manifest files found - applications run with user privileges but may fail when accessing protected resources.

## Implementation Plan

### 7.1 Add Application Manifest

Create manifest files for all executables:

```xml
<!-- Renegade.exe.manifest -->
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<assembly xmlns="urn:schemas-microsoft-com:asm.v1" manifestVersion="1.0">
  <assemblyIdentity
    version="1.0.37.0"
    processorArchitecture="x86"
    name="Renegade"
    type="win32"/>

  <description>Command &amp; Conquer Renegade</description>

  <!-- Request asInvoker - no elevation needed -->
  <trustInfo xmlns="urn:schemas-microsoft-com:asm.v3">
    <security>
      <requestedPrivileges>
        <requestedExecutionLevel level="asInvoker" uiAccess="false"/>
      </requestedPrivileges>
    </security>
  </trustInfo>

  <!-- Windows 10/11 compatibility -->
  <compatibility xmlns="urn:schemas-microsoft-com:compatibility.v1">
    <application>
      <!-- Windows 10/11 -->
      <supportedOS Id="{8e0f7a12-bfb3-4fe8-b9a5-48fd50a15a9a}"/>
      <!-- Windows 8.1 -->
      <supportedOS Id="{1f676c76-80e1-4239-95bb-83d0f6d0da78}"/>
      <!-- Windows 8 -->
      <supportedOS Id="{4a2f28e3-53b9-4441-ba9c-d69d4a4a6e38}"/>
      <!-- Windows 7 -->
      <supportedOS Id="{35138b9a-5d96-4fbd-8e2d-a2440225f93a}"/>
    </application>
  </compatibility>

  <!-- DPI awareness -->
  <application xmlns="urn:schemas-microsoft-com:asm.v3">
    <windowsSettings>
      <dpiAware xmlns="http://schemas.microsoft.com/SMI/2005/WindowsSettings">true</dpiAware>
      <dpiAwareness xmlns="http://schemas.microsoft.com/SMI/2016/WindowsSettings">
        PerMonitorV2
      </dpiAwareness>
    </windowsSettings>
  </application>
</assembly>
```

Installer requires admin - separate manifest:

```xml
<!-- Installer.exe.manifest -->
<trustInfo xmlns="urn:schemas-microsoft-com:asm.v3">
  <security>
    <requestedPrivileges>
      <requestedExecutionLevel level="requireAdministrator" uiAccess="false"/>
    </requestedPrivileges>
  </security>
</trustInfo>
```

### 7.2 Registry Migration

Move all game settings to `HKEY_CURRENT_USER`:

```cpp
// Code/wwlib/registry.cpp

// Old (requires admin):
// const char* REGISTRY_BASE = "Software\\Westwood\\Renegade";
// HKEY rootKey = HKEY_LOCAL_MACHINE;

// New (user-level):
const char* REGISTRY_BASE = "Software\\Westwood\\Renegade";
HKEY rootKey = HKEY_CURRENT_USER;

// Migration function - run once on first launch
void MigrateRegistrySettings() {
    // Check if old HKLM settings exist
    HKEY oldKey;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGISTRY_BASE, 0, KEY_READ, &oldKey) == ERROR_SUCCESS) {
        // Copy all values to HKCU
        HKEY newKey;
        RegCreateKeyEx(HKEY_CURRENT_USER, REGISTRY_BASE, 0, nullptr,
                       REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &newKey, nullptr);

        // Enumerate and copy values...
        CopyRegistryValues(oldKey, newKey);

        RegCloseKey(oldKey);
        RegCloseKey(newKey);

        // Mark migration complete
        WriteRegistryValue(HKEY_CURRENT_USER, REGISTRY_BASE, "Migrated", 1);
    }
}
```

### 7.3 User Data Directory

Move saves, configs, and logs to user-writable locations:

```cpp
// Code/Commando/userpaths.cpp

#include <shlobj.h>

class UserPaths {
public:
    static std::string GetSaveGamePath() {
        return GetUserDataPath() + "\\SaveGames";
    }

    static std::string GetConfigPath() {
        return GetUserDataPath() + "\\Config";
    }

    static std::string GetLogPath() {
        return GetUserDataPath() + "\\Logs";
    }

    static std::string GetScreenshotPath() {
        return GetUserDataPath() + "\\Screenshots";
    }

private:
    static std::string GetUserDataPath() {
        static std::string path;
        if (path.empty()) {
            char appData[MAX_PATH];
            if (SUCCEEDED(SHGetFolderPath(nullptr, CSIDL_LOCAL_APPDATA,
                                          nullptr, 0, appData))) {
                path = std::string(appData) + "\\Westwood\\Renegade";
                CreateDirectoryRecursive(path.c_str());
            }
        }
        return path;
    }
};
```

Updated paths:
| Old Location | New Location |
|--------------|--------------|
| `<GameDir>\Save\` | `%LOCALAPPDATA%\Westwood\Renegade\SaveGames\` |
| `<GameDir>\*.cfg` | `%LOCALAPPDATA%\Westwood\Renegade\Config\` |
| `<GameDir>\debug.log` | `%LOCALAPPDATA%\Westwood\Renegade\Logs\` |
| `<GameDir>\Screenshots\` | `%LOCALAPPDATA%\Westwood\Renegade\Screenshots\` |

### 7.4 Remove Privilege Escalation

Remove or guard shutdown privilege code:

```cpp
// Code/Launcher/patch.cpp

// Old: Escalates privileges to reboot
// void Shutdown_Computer_Now() { ... AdjustTokenPrivileges ... }

// New: Request reboot through proper channels
void RequestReboot() {
    // Just inform user, don't force reboot
    MessageBox(nullptr,
        "The update requires a system restart to complete.\n"
        "Please restart your computer when convenient.",
        "Restart Required",
        MB_OK | MB_ICONINFORMATION);

    // Or use ExitWindowsEx without privilege elevation (will prompt UAC)
    // ExitWindowsEx(EWX_REBOOT, SHTDN_REASON_MAJOR_APPLICATION);
}
```

### 7.5 Installer Modernization

Create a modern installer approach:

```cpp
// Modern installer options:

// Option A: Keep admin installer for Program Files
// - Installer.exe with requireAdministrator manifest
// - Installs to Program Files
// - Registers uninstaller

// Option B: Per-user installation
// - No admin required
// - Install to %LOCALAPPDATA%\Programs\Renegade
// - User-specific uninstall entry
```

For per-user installation:

```cpp
std::string GetPerUserInstallPath() {
    char localAppData[MAX_PATH];
    SHGetFolderPath(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, localAppData);
    return std::string(localAppData) + "\\Programs\\Renegade";
}

void RegisterPerUserUninstall() {
    // Write to HKCU instead of HKLM
    HKEY key;
    RegCreateKeyEx(HKEY_CURRENT_USER,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Renegade",
        0, nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &key, nullptr);

    RegSetValueEx(key, "DisplayName", 0, REG_SZ, ...);
    RegSetValueEx(key, "UninstallString", 0, REG_SZ, ...);
    // ...
    RegCloseKey(key);
}
```

### 7.6 VirtualStore Handling

Detect and migrate VirtualStore data:

```cpp
// Windows redirects writes to Program Files to VirtualStore
// %LOCALAPPDATA%\VirtualStore\Program Files\...

void MigrateVirtualStoreData() {
    char virtualStore[MAX_PATH];
    SHGetFolderPath(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, virtualStore);
    strcat(virtualStore, "\\VirtualStore");

    std::string oldGamePath = std::string(virtualStore) + "\\Program Files\\Westwood\\Renegade";

    if (DirectoryExists(oldGamePath.c_str())) {
        // Copy save games and configs to new location
        CopyDirectory((oldGamePath + "\\Save").c_str(), UserPaths::GetSaveGamePath().c_str());

        // Notify user
        MessageBox(nullptr,
            "Your save games have been migrated to a new location.",
            "Data Migration", MB_OK | MB_ICONINFORMATION);
    }
}
```

### 7.7 Remove MonoDrv Kernel Driver

The debug driver is incompatible with modern Windows:

```cpp
// Code/Tools/MonoDrv/ - DELETE ENTIRE DIRECTORY
// This kernel driver won't work on Windows 10/11 due to:
// - Driver signing requirements
// - Kernel patch protection
// - Secure Boot

// Replace with:
// - OutputDebugString() for debug output
// - Log file writing
// - Visual Studio debugger
```

### 7.8 DirectInput Fallback

Modern Windows may have DirectInput issues:

```cpp
// Add fallback to Raw Input if DirectInput fails
bool InitializeInput(HWND hwnd) {
    // Try DirectInput first
    if (DirectInputClass::Initialize(hwnd)) {
        return true;
    }

    // Fall back to Raw Input
    OutputDebugString("DirectInput failed, using Raw Input\n");
    return RawInputClass::Initialize(hwnd);
}
```

### 7.9 CMake Manifest Integration

```cmake
# Embed manifest in executable
if(MSVC)
    # Game executable - no elevation
    set_target_properties(Renegade PROPERTIES
        LINK_FLAGS "/MANIFEST:EMBED /MANIFESTINPUT:${CMAKE_SOURCE_DIR}/manifests/Renegade.exe.manifest"
    )

    # Installer - requires admin
    set_target_properties(Installer PROPERTIES
        LINK_FLAGS "/MANIFEST:EMBED /MANIFESTINPUT:${CMAKE_SOURCE_DIR}/manifests/Installer.exe.manifest"
    )
endif()
```

## File Changes Summary

| File | Change |
|------|--------|
| `wwlib/registry.cpp` | Use HKCU, add migration |
| `Commando/useroptions.cpp` | Use UserPaths for saves |
| `Commando/savegame.cpp` | Use UserPaths |
| `Launcher/patch.cpp` | Remove privilege escalation |
| `Installer/WinMain.cpp` | Per-user install option |
| `Tools/MonoDrv/*` | Delete |

## New Files

| File | Purpose |
|------|---------|
| `manifests/Renegade.exe.manifest` | UAC manifest |
| `manifests/Installer.exe.manifest` | Admin manifest |
| `Commando/userpaths.cpp` | User directory helpers |

## Testing Checklist

1. [ ] Game launches without admin prompt
2. [ ] Settings save correctly to HKCU
3. [ ] Save games work in user directory
4. [ ] Screenshots save to user directory
5. [ ] Config files migrate from old location
6. [ ] VirtualStore data migrates correctly
7. [ ] Installer works (with UAC prompt)
8. [ ] Game runs from Program Files (read-only)
9. [ ] Game runs from user-writable location
10. [ ] No UAC prompts during normal gameplay
11. [ ] Works with Windows Defender enabled
12. [ ] Works with standard user account (non-admin)

## Verification Commands

```powershell
# Check if UAC is triggering
Get-WinEvent -FilterHashtable @{LogName='Security'; Id=4688} |
    Where-Object { $_.Message -like '*Renegade*' }

# Check VirtualStore usage
Get-ChildItem "$env:LOCALAPPDATA\VirtualStore" -Recurse |
    Where-Object { $_.FullName -like '*Renegade*' }

# Verify registry location
reg query "HKCU\Software\Westwood\Renegade"
reg query "HKLM\Software\Westwood\Renegade"
```

## Estimated Effort

- Manifest creation: 0.5 day
- Registry migration: 2-3 days
- User paths implementation: 1-2 days
- Privilege removal: 0.5 day
- VirtualStore migration: 1 day
- Testing: 2-3 days
- **Total: 7-10 days**

## Risks

1. **Save game compatibility**: Old saves may have path issues
2. **Multiplayer configs**: Server configs may need admin for some features
3. **Mod support**: Mods expecting write access to game directory
4. **Antivirus false positives**: Changes may trigger new AV detections
