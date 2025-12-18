# Next Steps - Run from Developer Command Prompt

## Prerequisites

Open one of these:
- **x64 Native Tools Command Prompt for VS 2022**
- **Developer PowerShell for VS 2022**

Navigate to project root:
```cmd
cd /d D:\repos\FunNGames\CnC_Renegade
```

## Step 1: Configure CMake

```cmd
cmake -B build -G "Visual Studio 17 2022" -A x64
```

**Expected outcome**: CMake should configure successfully, creating `build/Renegade.sln`

**If errors occur**: Note the error messages - likely missing include directories or CMake syntax issues.

## Step 2: Build Debug Configuration

```cmd
cmake --build build --config Debug 2>&1 | tee build_debug.log
```

Or without tee (Windows cmd):
```cmd
cmake --build build --config Debug > build_debug.log 2>&1
```

**Expected outcome**: Many compilation errors due to legacy C++ code

**Save the log**: The `build_debug.log` file will contain all errors for analysis.

## Step 3: Report Errors

When you reopen Claude Code, share the first set of errors from `build_debug.log` and I'll help fix them.

Common expected errors:
1. Missing DirectX headers (d3d8.h, d3dx8.h)
2. Legacy C++ syntax (for loop variable scope, implicit int)
3. Deprecated functions (sprintf vs sprintf_s)
4. Missing Miles/Bink SDK headers

## Alternative: Build in Visual Studio

1. Open `build/Renegade.sln` in Visual Studio 2022
2. Set configuration to Debug/x64
3. Build Solution (Ctrl+Shift+B)
4. View errors in Error List window
5. Copy/paste errors for analysis

## Quick Commands Reference

```cmd
# Configure
cmake -B build -G "Visual Studio 17 2022" -A x64

# Build Debug
cmake --build build --config Debug

# Build Release
cmake --build build --config Release

# Clean and reconfigure
rmdir /s /q build
cmake -B build -G "Visual Studio 17 2022" -A x64

# Build specific target only
cmake --build build --config Debug --target wwlib
```

## Resume Point

When resuming with Claude Code:
1. Share contents of `build_debug.log` or first batch of errors
2. I'll analyze and create fixes for the compilation errors
3. We'll iterate until build succeeds
