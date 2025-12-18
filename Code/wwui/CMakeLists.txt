# wwui - UI Framework

file(GLOB WWUI_SOURCES "*.cpp" "*.c")
file(GLOB WWUI_HEADERS "*.h")

add_library(wwui STATIC ${WWUI_SOURCES} ${WWUI_HEADERS})

target_include_directories(wwui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/Code
)

target_link_libraries(wwui PUBLIC
    wwdebug
    wwlib
    wwmath
    ww3d2
)

# Source grouping for IDE
source_group("Source Files" FILES ${WWUI_SOURCES})
source_group("Header Files" FILES ${WWUI_HEADERS})
