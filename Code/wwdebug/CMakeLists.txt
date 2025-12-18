# wwdebug - Debug/logging utilities library
# Foundation library with no dependencies

file(GLOB WWDEBUG_SOURCES "*.cpp" "*.c")
file(GLOB WWDEBUG_HEADERS "*.h")

add_library(wwdebug STATIC ${WWDEBUG_SOURCES} ${WWDEBUG_HEADERS})

target_include_directories(wwdebug PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# No dependencies - this is the base library

# Source grouping for IDE
source_group("Source Files" FILES ${WWDEBUG_SOURCES})
source_group("Header Files" FILES ${WWDEBUG_HEADERS})
