# WWAudio - Audio system
# Uses OpenAL Soft for audio playback (replaces proprietary Miles Sound System)

file(GLOB WWAUDIO_SOURCES "*.cpp" "*.c")
file(GLOB WWAUDIO_HEADERS "*.h")

add_library(wwaudio STATIC ${WWAUDIO_SOURCES} ${WWAUDIO_HEADERS})

target_include_directories(wwaudio PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/Code
)

target_link_libraries(wwaudio PUBLIC
    wwdebug
    wwlib
    wwmath
    ww3d2
)

# Windows audio libraries (dsound no longer needed with OpenAL)
target_link_libraries(wwaudio PUBLIC
    winmm
)

# Source grouping for IDE
source_group("Source Files" FILES ${WWAUDIO_SOURCES})
source_group("Header Files" FILES ${WWAUDIO_HEADERS})
