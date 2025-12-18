# BandTest - Bandwidth testing library

file(GLOB BANDTEST_SOURCES "*.cpp" "*.c")
file(GLOB BANDTEST_HEADERS "*.h")

# Filter out resource files for library
list(FILTER BANDTEST_SOURCES EXCLUDE REGEX ".*\\.rc$")

add_library(bandtest STATIC ${BANDTEST_SOURCES} ${BANDTEST_HEADERS})

# Define BANDTEST_STATIC for static library build
target_compile_definitions(bandtest PUBLIC BANDTEST_STATIC)

target_include_directories(bandtest PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/Code
)

target_link_libraries(bandtest PUBLIC
    wwdebug
    wwlib
    wwnet
)

# Source grouping for IDE
source_group("Source Files" FILES ${BANDTEST_SOURCES})
source_group("Header Files" FILES ${BANDTEST_HEADERS})
