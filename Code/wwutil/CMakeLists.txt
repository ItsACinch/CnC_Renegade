# wwutil - Additional utilities

file(GLOB WWUTIL_SOURCES "*.cpp" "*.c")
file(GLOB WWUTIL_HEADERS "*.h")

add_library(wwutil STATIC ${WWUTIL_SOURCES} ${WWUTIL_HEADERS})

target_include_directories(wwutil PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/Code
)

target_link_libraries(wwutil PUBLIC
    wwdebug
    wwlib
)

# Source grouping for IDE
source_group("Source Files" FILES ${WWUTIL_SOURCES})
source_group("Header Files" FILES ${WWUTIL_HEADERS})
