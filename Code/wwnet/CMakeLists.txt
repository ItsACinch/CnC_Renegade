# wwnet - Networking library
# Packet management, sockets, NAT traversal

file(GLOB WWNET_SOURCES "*.cpp" "*.c")
file(GLOB WWNET_HEADERS "*.h")

add_library(wwnet STATIC ${WWNET_SOURCES} ${WWNET_HEADERS})

target_include_directories(wwnet PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/Code
)

target_link_libraries(wwnet PUBLIC
    wwdebug
    wwlib
)

# Windows networking libraries
target_link_libraries(wwnet PUBLIC
    wsock32
    ws2_32
)

# Source grouping for IDE
source_group("Source Files" FILES ${WWNET_SOURCES})
source_group("Header Files" FILES ${WWNET_HEADERS})
