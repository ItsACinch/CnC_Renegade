# wwlib - Foundation utilities library
# Memory, strings, file I/O, crypto (Blowfish), containers

file(GLOB WWLIB_SOURCES "*.cpp" "*.c")
file(GLOB WWLIB_HEADERS "*.h")

add_library(wwlib STATIC ${WWLIB_SOURCES} ${WWLIB_HEADERS})

target_include_directories(wwlib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/Code
)

target_link_libraries(wwlib PUBLIC
    wwdebug
)

# Preprocessor definitions from original .dsp
target_compile_definitions(wwlib PRIVATE
    REGEX_MALLOC
    STDC_HEADERS
)

# Source grouping for IDE
source_group("Source Files" FILES ${WWLIB_SOURCES})
source_group("Header Files" FILES ${WWLIB_HEADERS})
