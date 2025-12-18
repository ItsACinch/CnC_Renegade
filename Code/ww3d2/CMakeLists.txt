# ww3d2 - 3D Rendering Engine
# W3D format, DirectX 8 wrapper (to be migrated), mesh/animation systems

file(GLOB WW3D2_SOURCES "*.cpp" "*.c")
file(GLOB WW3D2_HEADERS "*.h" "*.inl")

# Exclude subdirectory test projects
list(FILTER WW3D2_SOURCES EXCLUDE REGEX ".*/hueshift/.*")
list(FILTER WW3D2_SOURCES EXCLUDE REGEX ".*/skeleton/.*")
list(FILTER WW3D2_SOURCES EXCLUDE REGEX ".*/sorttest/.*")

add_library(ww3d2 STATIC ${WW3D2_SOURCES} ${WW3D2_HEADERS})

target_include_directories(ww3d2 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/Code
    ${CMAKE_SOURCE_DIR}/Code/DirectX/include
)

target_link_libraries(ww3d2 PUBLIC
    wwdebug
    wwlib
    wwmath
)

# DirectX libraries (Windows SDK)
# D3D11 for modern rendering backend
target_link_libraries(ww3d2 PUBLIC
    d3d11
    dxgi
    dxguid
    dinput8
)

target_compile_definitions(ww3d2 PRIVATE
    DIRECTX
)

# Source grouping for IDE
source_group("Source Files" FILES ${WW3D2_SOURCES})
source_group("Header Files" FILES ${WW3D2_HEADERS})
