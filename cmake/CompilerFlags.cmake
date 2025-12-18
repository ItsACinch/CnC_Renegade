# CompilerFlags.cmake - Compiler-specific configuration for Renegade modernization

if(MSVC)
    # Warning level
    add_compile_options(/W3)  # Start with W3, increase later

    # Disable specific warnings for legacy code compatibility
    add_compile_options(
        /wd4996  # Deprecated functions (sprintf, etc.)
        /wd4244  # Conversion from 'type1' to 'type2', possible loss of data
        /wd4267  # Conversion from 'size_t' to 'type', possible loss of data
        /wd4018  # Signed/unsigned mismatch
        /wd4305  # Truncation from 'double' to 'float'
        /wd4800  # Forcing value to bool
        /wd4838  # Narrowing conversion
        /wd4101  # Unreferenced local variable
        /wd4102  # Unreferenced label
        /wd4146  # Unary minus operator applied to unsigned type
        /wd4477  # Format string mismatch
        /wd4312  # Conversion to greater size (casting)
        /wd4311  # Pointer truncation
        /wd4302  # Truncation
    )

    # Enable multi-processor compilation
    add_compile_options(/MP)

    # Use static runtime to match original build
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

    # Conformance mode (relaxed for legacy code)
    add_compile_options(/permissive)

    # MSVC 6.0 compatibility options
    # Allow for-loop variables to be visible outside the loop (old MSVC behavior)
    add_compile_options(/Zc:forScope-)
    # Treat wchar_t as unsigned short typedef (MSVC 6.0 behavior, not a built-in type)
    # This fixes wchar_t* to unsigned short* type mismatches in the codebase
    add_compile_options(/Zc:wchar_t-)
    # Disable two-phase name lookup (old behavior) - helps with template code
    add_compile_options(/Zc:twoPhase-)

    # Suppress deprecation warning for /Zc:forScope-
    add_compile_options(/wd4996)

    # Character set
    add_compile_options(/D_MBCS)

    # Exception handling
    add_compile_options(/EHsc)

    # Debug information
    add_compile_options($<$<CONFIG:Debug>:/ZI>)
    add_compile_options($<$<CONFIG:Release>:/Zi>)
    add_compile_options($<$<CONFIG:Profile>:/Zi>)

    # Optimization
    add_compile_options($<$<CONFIG:Debug>:/Od>)
    add_compile_options($<$<CONFIG:Release>:/O2>)
    add_compile_options($<$<CONFIG:Profile>:/O2>)

    # Inline expansion
    add_compile_options($<$<CONFIG:Release>:/Ob2>)
    add_compile_options($<$<CONFIG:Profile>:/Ob2>)

    # Linker options
    add_link_options($<$<CONFIG:Debug>:/DEBUG>)
    add_link_options($<$<CONFIG:Release>:/DEBUG>)  # Keep debug info even in release
    add_link_options($<$<CONFIG:Profile>:/DEBUG>)

    # Generate map files
    add_link_options(/MAP)

elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # GCC/Clang flags (for potential future cross-platform support)
    add_compile_options(-Wall -Wno-unused-variable -Wno-unused-but-set-variable)
    add_compile_options(-Wno-sign-compare -Wno-narrowing)

    add_compile_options($<$<CONFIG:Debug>:-g>)
    add_compile_options($<$<CONFIG:Debug>:-O0>)
    add_compile_options($<$<CONFIG:Release>:-O2>)
    add_compile_options($<$<CONFIG:Profile>:-O2>)
    add_compile_options($<$<CONFIG:Profile>:-g>)
endif()

# Print compiler info
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
