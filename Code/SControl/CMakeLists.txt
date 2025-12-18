# SControl - Server control socket interface

file(GLOB SCONTROL_SOURCES "*.cpp" "*.c")
file(GLOB SCONTROL_HEADERS "*.h")

add_library(scontrol STATIC ${SCONTROL_SOURCES} ${SCONTROL_HEADERS})

target_include_directories(scontrol PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/Code
)

target_link_libraries(scontrol PUBLIC
    wwdebug
    wwlib
    wwnet
)

# Source grouping for IDE
source_group("Source Files" FILES ${SCONTROL_SOURCES})
source_group("Header Files" FILES ${SCONTROL_HEADERS})
