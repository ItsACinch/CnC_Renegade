# wwbitpack - Bit packing for network compression

file(GLOB WWBITPACK_SOURCES "*.cpp" "*.c")
file(GLOB WWBITPACK_HEADERS "*.h")

add_library(wwbitpack STATIC ${WWBITPACK_SOURCES} ${WWBITPACK_HEADERS})

target_include_directories(wwbitpack PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/Code
)

target_link_libraries(wwbitpack PUBLIC
    wwdebug
    wwlib
)

# Source grouping for IDE
source_group("Source Files" FILES ${WWBITPACK_SOURCES})
source_group("Header Files" FILES ${WWBITPACK_HEADERS})
