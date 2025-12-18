# Command & Conquer Renegade - CMake Build System
# Modernization Phase 1: Build System Migration
cmake_minimum_required(VERSION 3.20)
project(Renegade VERSION 1.037 LANGUAGES CXX C)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build configurations
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;Profile" CACHE STRING "" FORCE)

# Profile = Release with debug info
set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_RELEASE}")
set(CMAKE_C_FLAGS_PROFILE "${CMAKE_C_FLAGS_RELEASE}")
set(CMAKE_EXE_LINKER_FLAGS_PROFILE "${CMAKE_EXE_LINKER_FLAGS_RELEASE}")
set(CMAKE_SHARED_LINKER_FLAGS_PROFILE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE}")
if(MSVC)
    string(APPEND CMAKE_CXX_FLAGS_PROFILE " /Zi")
    string(APPEND CMAKE_C_FLAGS_PROFILE " /Zi")
    string(APPEND CMAKE_EXE_LINKER_FLAGS_PROFILE " /DEBUG")
    string(APPEND CMAKE_SHARED_LINKER_FLAGS_PROFILE " /DEBUG")
endif()

# Output directories (matching original layout)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Run)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Run)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Code/Libs/${CMAKE_BUILD_TYPE})

# Per-configuration output directories
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG_UPPER)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} ${CMAKE_SOURCE_DIR}/Run)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} ${CMAKE_SOURCE_DIR}/Run)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} ${CMAKE_SOURCE_DIR}/Code/Libs/${OUTPUTCONFIG})
endforeach()

# CMake modules path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Include compiler configuration
include(CompilerFlags)

# Global definitions matching original build
add_compile_definitions(
    WIN32
    _WINDOWS
    WINVER=0x0601  # Windows 7 minimum
    _WIN32_WINNT=0x0601
    WIN32_LEAN_AND_MEAN
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Debug>:WWDEBUG>
    $<$<CONFIG:Profile>:NDEBUG>
    $<$<CONFIG:Profile>:WWDEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

# Options for special builds
option(BUILD_DEDICATED_SERVER "Build Free Dedicated Server" OFF)
option(BUILD_PUBLIC_EDITOR "Build Public Level Editor" OFF)
option(BUILD_TOOLS "Build development tools" OFF)
option(BUILD_TESTS "Build test applications" OFF)

if(BUILD_DEDICATED_SERVER)
    add_compile_definitions(FREEDEDICATEDSERVER)
endif()

if(BUILD_PUBLIC_EDITOR)
    add_compile_definitions(PUBLIC_EDITOR_VER)
endif()

# Include compatibility header globally
include_directories(${CMAKE_SOURCE_DIR}/Code/compat)

# Global include directories - original project had all library directories cross-visible
# This resolves circular header dependencies between libraries (wwdebug <-> wwlib, etc.)
include_directories(
    ${CMAKE_SOURCE_DIR}/Code
    ${CMAKE_SOURCE_DIR}/Code/wwdebug
    ${CMAKE_SOURCE_DIR}/Code/wwlib
    ${CMAKE_SOURCE_DIR}/Code/WWMath
    ${CMAKE_SOURCE_DIR}/Code/ww3d2
    ${CMAKE_SOURCE_DIR}/Code/wwphys
    ${CMAKE_SOURCE_DIR}/Code/wwnet
    ${CMAKE_SOURCE_DIR}/Code/wwsaveload
    ${CMAKE_SOURCE_DIR}/Code/wwtranslatedb
    ${CMAKE_SOURCE_DIR}/Code/wwbitpack
    ${CMAKE_SOURCE_DIR}/Code/wwui
    ${CMAKE_SOURCE_DIR}/Code/WWAudio
    ${CMAKE_SOURCE_DIR}/Code/wwutil
    ${CMAKE_SOURCE_DIR}/Code/BinkMovie
    ${CMAKE_SOURCE_DIR}/Code/SControl
    ${CMAKE_SOURCE_DIR}/Code/BandTest
    ${CMAKE_SOURCE_DIR}/Code/Combat
    ${CMAKE_SOURCE_DIR}/Code/Commando
    ${CMAKE_SOURCE_DIR}/Code/Scripts
    ${CMAKE_SOURCE_DIR}/Code/GameSpy
    ${CMAKE_SOURCE_DIR}/Code/wolapi
    ${CMAKE_SOURCE_DIR}/Code/WWOnline
)

# External dependencies
# DirectX SDK (June 2010) for legacy D3D9 compatibility
set(DIRECTX_SDK_DIR "${CMAKE_SOURCE_DIR}/DirectX-SDK-June2010-master")
if(EXISTS "${DIRECTX_SDK_DIR}/Include")
    message(STATUS "Found DirectX SDK at: ${DIRECTX_SDK_DIR}")
    include_directories(${DIRECTX_SDK_DIR}/Include)
    link_directories(${DIRECTX_SDK_DIR}/Lib/x86)
else()
    message(WARNING "DirectX SDK not found at ${DIRECTX_SDK_DIR}")
endif()

# =============================================================================
# Core Libraries (ordered by dependency)
# =============================================================================

# Foundation libraries (no game dependencies)
add_subdirectory(Code/wwdebug)
add_subdirectory(Code/wwlib)
add_subdirectory(Code/WWMath)

# Engine libraries
add_subdirectory(Code/ww3d2)
add_subdirectory(Code/wwphys)
add_subdirectory(Code/wwnet)
add_subdirectory(Code/wwsaveload)
add_subdirectory(Code/wwtranslatedb)
add_subdirectory(Code/wwbitpack)
add_subdirectory(Code/wwui)
add_subdirectory(Code/WWAudio)
add_subdirectory(Code/wwutil)

# Supporting libraries
add_subdirectory(Code/BinkMovie)
add_subdirectory(Code/SControl)
add_subdirectory(Code/BandTest)

# Game logic
add_subdirectory(Code/Combat)

# Stubs for deprecated dependencies (GameSpy)
add_subdirectory(Code/GameSpy)

# =============================================================================
# Main Executables and DLLs
# =============================================================================

add_subdirectory(Code/Scripts)
add_subdirectory(Code/Commando)

# =============================================================================
# Optional: Tools
# =============================================================================

if(BUILD_TOOLS)
    # add_subdirectory(Code/Tools/LevelEdit)
    # add_subdirectory(Code/Tools/W3DView)
    # add_subdirectory(Code/Tools/MixViewer)
    # etc.
    message(STATUS "Tools build not yet implemented")
endif()

# =============================================================================
# Optional: Tests
# =============================================================================

if(BUILD_TESTS)
    # add_subdirectory(Code/Tests/mathtest)
    # add_subdirectory(Code/Tests/PhysTest)
    # etc.
    message(STATUS "Tests build not yet implemented")
endif()

# =============================================================================
# Runtime DLL Dependencies
# =============================================================================
# TODO: Automate copying of runtime DLL dependencies to the output directory.
#
# Currently, users must manually download and copy these DLLs:
#
# 1. OpenAL Soft (Audio):
#    - Download from: https://openal-soft.org/openal-binaries/
#    - Copy soft_oal.dll as OpenAL32.dll to Run/
#
# 2. FFmpeg (Video):
#    - Download from: https://www.gyan.dev/ffmpeg/builds/ (shared build)
#    - Copy avcodec-XX.dll, avformat-XX.dll, avutil-XX.dll, swscale-XX.dll to Run/
#
# Future improvements:
# - Use FetchContent or ExternalProject to download OpenAL and FFmpeg during configure
# - Use find_package() with custom Find modules to locate installed libraries
# - Add post-build commands to copy DLLs automatically:
#   add_custom_command(TARGET renegade POST_BUILD
#       COMMAND ${CMAKE_COMMAND} -E copy_if_different
#       "${OPENAL_DLL}" "$<TARGET_FILE_DIR:renegade>"
#   )
# - Consider using CPack to bundle DLLs with the game for distribution
# - Alternatively, use vcpkg or conan for dependency management
#
# Example FetchContent approach (untested):
# include(FetchContent)
# FetchContent_Declare(
#     openal-soft
#     URL https://openal-soft.org/openal-binaries/openal-soft-1.23.1-bin.zip
# )
# FetchContent_MakeAvailable(openal-soft)

# =============================================================================
# Installation (future)
# =============================================================================

# install(TARGETS Renegade Scripts
#     RUNTIME DESTINATION bin
#     LIBRARY DESTINATION bin
# )
