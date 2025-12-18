# BinkMovie - Video playback
# RAD Bink SDK wrapper (to be replaced with FFmpeg in Phase 4)

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

# Bink library (Phase 4 will replace with FFmpeg)
if(EXISTS "${CMAKE_SOURCE_DIR}/Code/BinkMovie/binkw32.lib")
    target_link_libraries(binkmovie PUBLIC
        ${CMAKE_SOURCE_DIR}/Code/BinkMovie/binkw32.lib
    )
else()
    message(WARNING "Bink SDK not found - video playback will be stubbed")
    target_compile_definitions(binkmovie PUBLIC VIDEO_STUB)
endif()

# Video for Windows (fallback)
target_link_libraries(binkmovie PUBLIC
    vfw32
)

# Source grouping for IDE
source_group("Source Files" FILES ${BINKMOVIE_SOURCES})
source_group("Header Files" FILES ${BINKMOVIE_HEADERS})
