# Combat - Game logic library
# Game objects, weapons, vehicles, soldiers, buildings, AI, damage system, scripting

file(GLOB COMBAT_SOURCES "*.cpp" "*.c")
file(GLOB COMBAT_HEADERS "*.h")

add_library(combat STATIC ${COMBAT_SOURCES} ${COMBAT_HEADERS})

target_include_directories(combat PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/Code
    ${CMAKE_SOURCE_DIR}/Code/DirectX/include
    ${CMAKE_SOURCE_DIR}/Code/Miles6/include
)

target_link_libraries(combat PUBLIC
    wwdebug
    wwlib
    wwmath
    ww3d2
    wwphys
    wwnet
    wwsaveload
    wwtranslatedb
    wwbitpack
    wwui
    wwaudio
    wwutil
    binkmovie
    scontrol
)

target_compile_definitions(combat PRIVATE
    DIRECTX
)

# Source grouping for IDE
source_group("Source Files" FILES ${COMBAT_SOURCES})
source_group("Header Files" FILES ${COMBAT_HEADERS})
