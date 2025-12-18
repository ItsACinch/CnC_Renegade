# WWMath - Math library
# Vectors, matrices, quaternions, collision primitives

file(GLOB WWMATH_SOURCES "*.cpp" "*.c")
file(GLOB WWMATH_HEADERS "*.h" "*.inl")

add_library(wwmath STATIC ${WWMATH_SOURCES} ${WWMATH_HEADERS})

target_include_directories(wwmath PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/Code
)

target_link_libraries(wwmath PUBLIC
    wwdebug
    wwlib
)

# Source grouping for IDE
source_group("Source Files" FILES ${WWMATH_SOURCES})
source_group("Header Files" FILES ${WWMATH_HEADERS})
