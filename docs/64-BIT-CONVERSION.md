# 64-bit Conversion Plan for Command & Conquer Renegade

## Overview

Convert the Win32 (x86) codebase to x64 to enable use of modern 64-bit libraries (FFmpeg, OpenAL) without needing legacy 32-bit builds.

**Motivation**: FFmpeg only provides 64-bit DLLs in current releases. Licensing prevents redistributing converted video files, so the game must read original Bink files via FFmpeg, requiring native x64.

**Note**: DirectX 8 has been upgraded to DirectX 9 using a translation layer, so DX compatibility is not a concern.

---

## Phase 1: Foundation Types (wwlib)

### 1.1 Fix bittype.h Type Definitions [DONE]

**File**: `Code/wwlib/bittype.h`

Replace platform-dependent types with fixed-width types:
```cpp
// Change from:
typedef unsigned long  uint32;
typedef long           sint32;

// Change to:
#include <stdint.h>
typedef uint32_t  uint32;
typedef int32_t   sint32;
typedef uintptr_t uintptr;  // New: for pointer-sized integers
typedef intptr_t  sintptr;  // New: for pointer-sized integers
```

### 1.2 Fix Pointer-to-Integer Casts in vector.h [DONE]

**File**: `Code/wwlib/vector.h`

The `ID()` function casts pointers to unsigned long:
```cpp
// Change from:
unsigned long ID(T const * pointer) const {
    return (((unsigned long)pointer) - ((unsigned long)&(*this)[0])) / sizeof(T);
}

// Change to:
size_t ID(T const * pointer) const {
    return (((uintptr_t)pointer) - ((uintptr_t)&(*this)[0])) / sizeof(T);
}
```

### 1.3 Fix Memory Pool (mempool.h) [DONE]

**File**: `Code/wwlib/mempool.h`

BlockListHead uses `uint32*` to store block pointers:
```cpp
// Change from:
uint32* BlockListHead;
*(uint32*)block = (uint32)BlockListHead;

// Change to:
void* BlockListHead;
*(void**)block = BlockListHead;
```

### 1.4 Fix FastAllocator.h

**File**: `Code/wwlib/FastAllocator.h`

Link structure may have alignment/size assumptions. Audit all `sizeof(Link*)` usage.

### 1.5 Fix smartptr.h

**File**: `Code/wwlib/smartptr.h`

Remove dangerous pointer-to-long cast:
```cpp
// Remove or change:
operator long() { return (long)Pointer; }

// To explicit method:
T* Get() const { return Pointer; }
```

---

## Phase 2: Serialization System (wwsaveload)

### 2.1 Fix PersistFactory Pointer Serialization

**File**: `Code/wwsaveload/persistfactory.h`

Critical issue - pointers are cast to uint32 for serialization:
```cpp
// Change from:
uint32 objptr = (uint32)obj;
cstm.Write(&objptr, sizeof(objptr));

// Change to (use ID-based approach):
uint32 obj_id = Get_Object_ID(obj);  // Map pointer to stable ID
cstm.Write(&obj_id, sizeof(obj_id));
```

**Save File Compatibility**: This WILL break compatibility with existing 32-bit save files. Options:
1. Version the save format and support both
2. Accept save incompatibility for 64-bit builds
3. Create migration tool

### 2.2 Audit WRITE_MICRO_CHUNK Usage

**Files affected** (20+ files in Code/Combat/):
- soldier.cpp, vehicle.cpp, building.cpp
- weapons.cpp, weaponbag.cpp
- physicalgameobj.cpp, damageablegameobj.cpp
- And many more...

Each use of WRITE_MICRO_CHUNK with pointer data must be audited. Most are likely writing object references that need ID-based serialization.

---

## Phase 3: Inline Assembly Replacement

### 3.1 CPU Detection (cpudetect.cpp)

**File**: `Code/wwlib/cpudetect.cpp`

Replace x86 CPUID assembly with compiler intrinsics:
```cpp
// Change from:
__asm {
    mov eax, 1
    cpuid
    mov features, edx
}

// Change to:
#include <intrin.h>
int cpuInfo[4];
__cpuid(cpuInfo, 1);
features = cpuInfo[3];
```

### 3.2 LCW Compression (lcw.cpp)

**File**: `Code/wwlib/lcw.cpp`

Contains optimized x86 assembly for LCW decompression. Options:
1. Use C fallback (already exists, enable it)
2. Write x64 assembly version
3. Use modern compression library

Recommend: Enable C fallback for initial port, optimize later if needed.

### 3.3 Exception Handling (Except.cpp)

**File**: `Code/wwlib/Except.cpp`

x86 SEH assembly. Replace with:
```cpp
// Use __try/__except with compiler intrinsics
// Or use C++ exceptions with proper stack unwinding
```

### 3.4 VerStamp Tool (VerStamp.asm)

**File**: `Code/Tools/VerStamp/VerStamp.asm`

1235 lines of pure x86 assembly. Options:
1. Rewrite in C/C++ (significant effort)
2. Keep as separate 32-bit tool (doesn't need to match game architecture)
3. Replace with modern version stamping approach

**Recommendation**: Keep as 32-bit tool - it's a build tool, not runtime code.

---

## Phase 4: Windows API Updates

### 4.1 SetWindowLong/GetWindowLong

**Files affected**: Multiple window handling files

```cpp
// Change from:
SetWindowLong(hwnd, GWL_USERDATA, (LONG)this);
MyClass* ptr = (MyClass*)GetWindowLong(hwnd, GWL_USERDATA);

// Change to:
SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);
MyClass* ptr = (MyClass*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
```

### 4.2 WNDPROC Casting

Ensure window procedure casts use proper 64-bit safe macros.

### 4.3 Message Parameter Handling

WPARAM and LPARAM are pointer-sized on x64. Audit any code that treats them as 32-bit.

---

## Phase 5: Build System

### 5.1 CMake Platform Configuration

Update CMakeLists.txt:
```cmake
# Add x64 platform support
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    add_compile_definitions(WIN64 _WIN64)
    set(PLATFORM_DIR "x64")
else()
    set(PLATFORM_DIR "x86")
endif()

# Update library paths
link_directories(${DIRECTX_SDK_DIR}/Lib/${PLATFORM_DIR})
```

### 5.2 Conditional Compilation

Add guards for platform-specific code:
```cpp
#ifdef _WIN64
    // 64-bit implementation
#else
    // 32-bit implementation (legacy)
#endif
```

---

## Phase 6: Testing Strategy

### 6.1 Unit Tests for Critical Systems
- Memory allocator tests
- Serialization round-trip tests
- Math library precision tests

### 6.2 Integration Testing
- Load existing game levels
- Play through missions
- Verify multiplayer functionality

### 6.3 Save File Testing
- Document incompatibility clearly
- Test new save/load cycle on x64

---

## Implementation Order

1. **Week 1-2**: Phase 1 (Foundation Types)
   - Fix bittype.h, vector.h, mempool.h, smartptr.h
   - Get clean compile on x64

2. **Week 3-4**: Phase 3 & 4 (Assembly & Windows API)
   - Replace inline assembly with intrinsics/C
   - Update Windows API calls

3. **Week 5-6**: Phase 2 (Serialization)
   - Implement ID-based object references
   - Accept save file incompatibility

4. **Week 7-8**: Phase 5 & 6 (Build & Test)
   - Update CMake for x64
   - Integration testing

---

## Risk Assessment

| Risk | Severity | Mitigation |
|------|----------|------------|
| Save file incompatibility | Medium | Document clearly, version the format |
| Performance regression (assembly removal) | Low | Profile and optimize hot paths |
| Hidden pointer truncation bugs | High | Static analysis tools, thorough testing |
| DirectX 8 x64 compatibility | N/A | Already upgraded to DX9 with translation layer |

---

## Files Summary

**Must Change** (blocking compilation):
- Code/wwlib/bittype.h [DONE] - Fixed types, include windows.h for Windows types
- Code/wwlib/vector.h [DONE] - Use uintptr_t for pointer arithmetic
- Code/wwlib/mempool.h [DONE] - Use void* for block pointers
- Code/wwlib/cpudetect.cpp [DONE] - Use __cpuid intrinsic
- Code/wwlib/lcw.cpp [DONE] - Conditional compilation for x86 assembly
- Code/wwlib/Except.cpp [DONE] - Conditional compilation for x86 assembly
- Code/wwlib/blitblit.h [DONE] - Guard x86 assembly with !defined(_M_X64)
- Code/wwlib/rlerle.h [DONE] - Guard x86 assembly with !defined(_M_X64)
- Code/wwlib/mpu.cpp [DONE] - Replace RDTSC assembly with __rdtsc() intrinsic
- Code/wwlib/wwmouse.cpp [DONE] - Fix timer callback to use DWORD_PTR
- Code/wwnet/packetmgr.cpp [DONE] - Replace bswap assembly with _byteswap_ulong()
- Code/wwphys/vistable.cpp [DONE] - Use DWORD for ReadFile/WriteFile parameters
- Code/commando/datasafe.h [DONE] - Replace label-address assembly with static variable address
- Code/Commando/WINMAIN.CPP [DONE] - Use LRESULT CALLBACK for WNDPROC

**Should Change** (runtime correctness):
- Code/wwlib/smartptr.h [DONE] - operator long removed/changed
- Code/wwlib/FastAllocator.h
- Code/wwsaveload/persistfactory.h
- 20+ Combat/*.cpp files with WRITE_MICRO_CHUNK
- All SetWindowLong/GetWindowLong usages

**Can Keep 32-bit**:
- Code/Tools/VerStamp/VerStamp.asm (build tool)

---

## Current Status: **BUILD SUCCESSFUL**

The x64 build now compiles and links successfully. The executable is output to `Run/Renegade.exe`.

Key changes made:
1. All x86 inline assembly guarded with `#if !defined(_M_X64)` or replaced with intrinsics
2. Windows types obtained from `<windows.h>` in bittype.h
3. RDTSC instructions replaced with `__rdtsc()` intrinsic
4. BSWAP instructions replaced with `_byteswap_ulong()`
5. Window procedure signatures updated to use `LRESULT CALLBACK`
6. Timer callbacks updated to use `DWORD_PTR` parameters
