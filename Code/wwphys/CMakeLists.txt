# wwphys - Physics Engine
# Rigid bodies, collision detection, vehicles, projectiles

file(GLOB WWPHYS_SOURCES "*.cpp" "*.c")
file(GLOB WWPHYS_HEADERS "*.h")

# Exclude editor-specific files that require LevelEdit headers
list(FILTER WWPHYS_SOURCES EXCLUDE REGEX "PathfindSectorBuilder\\.cpp$")

add_library(wwphys STATIC ${WWPHYS_SOURCES} ${WWPHYS_HEADERS})

target_include_directories(wwphys PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/Code
)

target_link_libraries(wwphys PUBLIC
    wwdebug
    wwlib
    wwmath
    ww3d2
)

# Source grouping for IDE
source_group("Source Files" FILES ${WWPHYS_SOURCES})
source_group("Header Files" FILES ${WWPHYS_HEADERS})
