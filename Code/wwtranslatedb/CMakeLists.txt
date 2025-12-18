# wwtranslatedb - Localization/string database

file(GLOB WWTRANSLATEDB_SOURCES "*.cpp" "*.c")
file(GLOB WWTRANSLATEDB_HEADERS "*.h")

add_library(wwtranslatedb STATIC ${WWTRANSLATEDB_SOURCES} ${WWTRANSLATEDB_HEADERS})

target_include_directories(wwtranslatedb PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/Code
)

target_link_libraries(wwtranslatedb PUBLIC
    wwdebug
    wwlib
)

# Source grouping for IDE
source_group("Source Files" FILES ${WWTRANSLATEDB_SOURCES})
source_group("Header Files" FILES ${WWTRANSLATEDB_HEADERS})
