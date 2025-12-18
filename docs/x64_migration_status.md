# x64 Migration Status

## Current State: BUILD SUCCEEDS, RUNTIME CRASH

The x64 build compiles successfully but crashes before WinMain is reached (no diagnostic log file is generated).

## Completed Fixes

### 1. Inline Assembly Replacements

| File | Issue | Fix |
|------|-------|-----|
| `wwlib/blitblit.h` | x86 assembly | Added `#if !defined(_M_X64)` guard |
| `wwlib/rlerle.h` | x86 assembly | Added `#if !defined(_M_X64)` guard |
| `wwlib/mpu.cpp` | RDTSC assembly | Replaced with `__rdtsc()` intrinsic |
| `wwlib/lcw.cpp` | x86 assembly | Already had `#ifndef _WIN64` guard |
| `wwnet/packetmgr.cpp` | BSWAP assembly | Replaced with `_byteswap_ulong()` |
| `wwdebug/wwprofile.cpp` | RDTSC assembly | Replaced with `__rdtsc()` intrinsic |
| `wwlib/Except.cpp` | Stack walk assembly | Already had proper `#ifdef _WIN64` guards |

### 2. Pointer Size Fixes

| File | Issue | Fix |
|------|-------|-----|
| `wwlib/wwmouse.cpp` | Timer callback DWORD | Changed to `DWORD_PTR` |
| `wwphys/vistable.cpp` | ReadFile/WriteFile param | Changed `uint32` to `DWORD` |
| `Commando/WINMAIN.CPP` | Window proc signature | Changed to `LRESULT CALLBACK` |
| `wwlib/mempool.h` | BlockListHead type | Changed from `uint32*` to `void*` |
| `wwlib/smartptr.h` | Integer conversion | Uses `intptr_t` for 64-bit |

### 3. DataSafe Type Code Collision (Critical Fix)

**File:** `commando/datasafe.h` and `commando/datasafe.cpp`

**Problem:** Original code used assembly label addresses truncated to 32-bit as type identifiers. On x64, different types could have the same lower 32 bits, causing collisions and infinite loops.

**Fix:** Added a global counter `NextTypeCode` in `GenericDataSafeClass` that assigns unique IDs to each template instantiation:

```cpp
// In datasafe.h - class GenericDataSafeClass
static unsigned long NextTypeCode;

// In datasafe.cpp
unsigned long GenericDataSafeClass::NextTypeCode = 1;

// In Get_Type_Code() template function
static unsigned long type_id = 0;
if (type_id == 0) {
    type_id = NextTypeCode++;
}
return type_id;
```

### 4. PersistFactory Pointer Size Mismatch (Critical Fix)

**File:** `wwsaveload/persistfactory.h`

**Problem:** Save writes 4-byte pointer ID, but Load was reading `sizeof(T*)` which is 8 bytes on x64.

**Fix:** Conditional compilation to read 4 bytes and extend to pointer:

```cpp
#ifdef _M_X64
    uint32 old_obj_id = 0;
    cload.Read(&old_obj_id, sizeof(uint32));
    old_obj = (T*)(uintptr_t)old_obj_id;
#else
    cload.Read(&old_obj,sizeof(T *));
#endif
```

## Current Issue: Pre-WinMain Crash

### Symptoms
- Build succeeds without errors
- Executable launches but immediately crashes
- No diagnostic log file is created in `%TEMP%\renegade_diag.txt`
- This indicates crash during static initialization or DLL loading, before WinMain

### Diagnostic Logging Added
Added early logging at the very start of WinMain in `WINMAIN.CPP`:
- Creates `%TEMP%\renegade_diag.txt` immediately on entry
- Uses `OutputDebugStringA` for debugger output
- If file is not created, crash is BEFORE WinMain

### Potential Causes to Investigate

1. **Static Initialization Order**
   - Many global objects are constructed before WinMain
   - `DataSafeClass<T>` instances created via `DECLARE_DATA_SAFE` macro
   - `SimplePersistFactoryClass` instances for serialization
   - `DynamicVectorClass` and other containers

2. **DLL Loading**
   - Missing x64 DLL dependencies
   - Check with Dependency Walker or `dumpbin /dependents`

3. **Remaining Assembly**
   - Several files still have assembly that may need guards:
     - `WWMath/vp.cpp` - SSE assembly macros
     - `WWMath/matrix3d.cpp` - Some assembly
     - `WWMath/quat.cpp` - FPU assembly
     - `ww3d2/dx8wrapper.h` - Assembly blocks

4. **Pointer Truncation**
   - Search for `(uint32)` or `(DWORD)` casts applied to pointers
   - Check for `unsigned long` used to store pointer values

## Files Modified in This Session

1. `Code/commando/datasafe.h` - Type code fix
2. `Code/commando/datasafe.cpp` - NextTypeCode static member
3. `Code/wwsaveload/persistfactory.h` - x64 conditional read
4. `Code/Commando/WINMAIN.CPP` - Early diagnostic logging + diaglog.h include

## Build Command

```
"C:/Program Files/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin/amd64/MSBuild.exe" ^
  "D:/repos/FunNGames/CnC_Renegade/build_x64/Renegade.sln" ^
  -t:renegade -p:Configuration=Release -v:minimal
```

## Next Steps

1. **Debug the pre-WinMain crash:**
   - Use Visual Studio debugger to catch the crash
   - Check Windows Event Viewer for crash details
   - Use `dumpbin /dependents Renegade.exe` to check DLL deps

2. **Fix remaining assembly:**
   - Add x64 guards to WWMath assembly files
   - Provide C++ fallbacks where needed

3. **Test with debugger attached:**
   - Set exception breakpoints to catch first-chance exceptions
   - Check CRT initialization

4. **Consider static init order:**
   - May need `#pragma init_seg` adjustments
   - Check if some static objects depend on others not yet constructed

## Reference: Assembly Files Needing Attention

```
WWMath/wwmath.h     - Has proper guards (#if defined(_MSC_VER) && defined(_M_IX86))
WWMath/vp.cpp       - SSE macros using __asm - NEEDS GUARD
WWMath/matrix3d.cpp - Some assembly - NEEDS CHECK
WWMath/quat.cpp     - FPU assembly - NEEDS CHECK
ww3d2/dx8wrapper.h  - Assembly blocks - NEEDS CHECK
```
