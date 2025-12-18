# BinkMovie - Video playback
# Uses FFmpeg as a replacement for RAD Bink SDK
# FFmpeg DLLs are loaded dynamically at runtime

file(GLOB BINKMOVIE_SOURCES "*.cpp" "*.c")
file(GLOB BINKMOVIE_HEADERS "*.h")

add_library(binkmovie STATIC ${BINKMOVIE_SOURCES} ${BINKMOVIE_HEADERS})

target_include_directories(binkmovie PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/Code
)

target_link_libraries(binkmovie PUBLIC
    wwdebug
    wwlib
    ww3d2
)

# Video for Windows (fallback/utility functions)
target_link_libraries(binkmovie PUBLIC
    vfw32
)

# Source grouping for IDE
source_group("Source Files" FILES ${BINKMOVIE_SOURCES})
source_group("Header Files" FILES ${BINKMOVIE_HEADERS})
