# wwsaveload - Serialization/chunk-based file format system

file(GLOB WWSAVELOAD_SOURCES "*.cpp" "*.c")
file(GLOB WWSAVELOAD_HEADERS "*.h")

add_library(wwsaveload STATIC ${WWSAVELOAD_SOURCES} ${WWSAVELOAD_HEADERS})

target_include_directories(wwsaveload PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/Code
)

target_link_libraries(wwsaveload PUBLIC
    wwdebug
    wwlib
)

# Source grouping for IDE
source_group("Source Files" FILES ${WWSAVELOAD_SOURCES})
source_group("Header Files" FILES ${WWSAVELOAD_HEADERS})
