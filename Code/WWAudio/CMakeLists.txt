# WWAudio - Audio system
# Miles Sound System integration (to be replaced with OpenAL in Phase 3)

file(GLOB WWAUDIO_SOURCES "*.cpp" "*.c")
file(GLOB WWAUDIO_HEADERS "*.h")

add_library(wwaudio STATIC ${WWAUDIO_SOURCES} ${WWAUDIO_HEADERS})

target_include_directories(wwaudio PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/Code
    ${CMAKE_SOURCE_DIR}/Code/Miles6/include  # Original Miles SDK path
)

target_link_libraries(wwaudio PUBLIC
    wwdebug
    wwlib
    wwmath
    ww3d2
)

# Miles Sound System library (Phase 3 will replace with OpenAL)
# For now, link against stub or skip if not available
if(EXISTS "${CMAKE_SOURCE_DIR}/Code/Miles6/lib/win/mss32.lib")
    target_link_libraries(wwaudio PUBLIC
        ${CMAKE_SOURCE_DIR}/Code/Miles6/lib/win/mss32.lib
    )
else()
    message(WARNING "Miles Sound System SDK not found - audio will be stubbed")
    target_compile_definitions(wwaudio PUBLIC AUDIO_STUB)
endif()

# Windows audio libraries
target_link_libraries(wwaudio PUBLIC
    dsound
    winmm
)

# Source grouping for IDE
source_group("Source Files" FILES ${WWAUDIO_SOURCES})
source_group("Header Files" FILES ${WWAUDIO_HEADERS})
