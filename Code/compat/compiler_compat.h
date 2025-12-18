/*
 * compiler_compat.h - Compatibility layer for modern MSVC
 *
 * This header provides compatibility definitions to allow the legacy
 * Renegade codebase to compile with modern compilers (MSVC 2019/2022+).
 *
 * Include this header first in any translation unit that has issues
 * with modern compiler standards.
 */

#pragma once

#ifndef COMPILER_COMPAT_H
#define COMPILER_COMPAT_H

// =============================================================================
// Compiler Detection
// =============================================================================

#if defined(_MSC_VER)
    #define RENEGADE_MSVC 1
    #if _MSC_VER >= 1920  // VS 2019+
        #define RENEGADE_MODERN_MSVC 1
    #endif
    #if _MSC_VER >= 1930  // VS 2022+
        #define RENEGADE_MSVC_2022 1
    #endif
#elif defined(__GNUC__)
    #define RENEGADE_GCC 1
#elif defined(__clang__)
    #define RENEGADE_CLANG 1
#endif

// =============================================================================
// Deprecated Keyword Handling
// =============================================================================

// The 'register' keyword was deprecated in C++11 and removed in C++17
// Original code used it for performance hints
#define register

// =============================================================================
// Exception Specification Compatibility
// =============================================================================

// throw() specifications were deprecated in C++11 and removed in C++17
// Modern code should use noexcept instead
#define THROW_SPEC(x)
#define THROW_NONE noexcept

// Legacy throw() macro for headers that used it
#ifdef RENEGADE_MODERN_MSVC
    #define throw()
#endif

// =============================================================================
// Smart Pointer Compatibility
// =============================================================================

// auto_ptr was removed in C++17 - redirect to unique_ptr where possible
// Note: This is a rough compatibility shim; behavior differs
#include <memory>
#if __cplusplus >= 201703L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L)
    // In C++17 mode, auto_ptr doesn't exist
    // Code using auto_ptr should be manually updated
    #define AUTO_PTR_DEPRECATED 1
#endif

// =============================================================================
// String Function Compatibility
// =============================================================================

// Disable deprecation warnings for standard C functions
#ifndef _CRT_SECURE_NO_WARNINGS
    #define _CRT_SECURE_NO_WARNINGS
#endif

#ifndef _CRT_NONSTDC_NO_WARNINGS
    #define _CRT_NONSTDC_NO_WARNINGS
#endif

// Provide stricmp/strnicmp if not available
#ifdef RENEGADE_MODERN_MSVC
    #ifndef stricmp
        #define stricmp _stricmp
    #endif
    #ifndef strnicmp
        #define strnicmp _strnicmp
    #endif
    #ifndef strupr
        #define strupr _strupr
    #endif
    #ifndef strlwr
        #define strlwr _strlwr
    #endif
    #ifndef itoa
        #define itoa _itoa
    #endif
    #ifndef ltoa
        #define ltoa _ltoa
    #endif
#endif

// =============================================================================
// Windows Header Compatibility
// =============================================================================

// On x64, intrin.h MUST be included before windows.h to avoid
// duplicate declaration errors for intrinsics like __readgsqword
#ifdef _WIN64
    #include <intrin.h>
#endif

// Ensure proper Windows version targeting
#ifndef WINVER
    #define WINVER 0x0601  // Windows 7
#endif

#ifndef _WIN32_WINNT
    #define _WIN32_WINNT 0x0601
#endif

// Reduce Windows header bloat
#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
    #define NOMINMAX  // Prevent Windows.h from defining min/max macros
#endif

// =============================================================================
// Type Compatibility
// =============================================================================

// Ensure BOOL is available
#ifndef BOOL
    typedef int BOOL;
#endif

#ifndef TRUE
    #define TRUE 1
#endif

#ifndef FALSE
    #define FALSE 0
#endif

// =============================================================================
// Inline/Forceinline Compatibility
// =============================================================================

#ifdef RENEGADE_MSVC
    #define FORCE_INLINE __forceinline
#elif defined(RENEGADE_GCC) || defined(RENEGADE_CLANG)
    #define FORCE_INLINE __attribute__((always_inline)) inline
#else
    #define FORCE_INLINE inline
#endif

// =============================================================================
// Alignment Compatibility
// =============================================================================

#ifdef RENEGADE_MSVC
    #define ALIGN(x) __declspec(align(x))
#else
    #define ALIGN(x) __attribute__((aligned(x)))
#endif

// =============================================================================
// DLL Export/Import
// =============================================================================

#ifdef RENEGADE_MSVC
    #define DLL_EXPORT __declspec(dllexport)
    #define DLL_IMPORT __declspec(dllimport)
#else
    #define DLL_EXPORT __attribute__((visibility("default")))
    #define DLL_IMPORT
#endif

// =============================================================================
// Debug Break
// =============================================================================

#ifdef RENEGADE_MSVC
    #define DEBUG_BREAK() __debugbreak()
#elif defined(RENEGADE_GCC) || defined(RENEGADE_CLANG)
    #define DEBUG_BREAK() __builtin_trap()
#else
    #define DEBUG_BREAK() ((void)0)
#endif

// =============================================================================
// Deprecated Features Warning Suppression
// =============================================================================

#ifdef RENEGADE_MSVC
    // Suppress specific warnings globally for legacy code
    #pragma warning(disable: 4996)  // Deprecated functions
    #pragma warning(disable: 4244)  // Conversion warnings
    #pragma warning(disable: 4267)  // size_t to int
    #pragma warning(disable: 4302)  // Truncation
    #pragma warning(disable: 4311)  // Pointer truncation
    #pragma warning(disable: 4312)  // Conversion to greater size
#endif

#endif // COMPILER_COMPAT_H
