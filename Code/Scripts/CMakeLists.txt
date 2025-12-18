# Scripts - DLL-based mission scripting system
# Scripts inherit from ScriptImpClass and use ScriptRegistrant for registration

file(GLOB SCRIPTS_SOURCES "*.cpp" "*.c")
file(GLOB SCRIPTS_HEADERS "*.h")

# Exclude mission scripts (require default argument fix for function pointers)
# These can be re-added once all call sites are updated
list(FILTER SCRIPTS_SOURCES EXCLUDE REGEX "[Mm]ission[0-9]+\\.cpp$")
list(FILTER SCRIPTS_SOURCES EXCLUDE REGEX "MissionDemo\\.cpp$")
list(FILTER SCRIPTS_SOURCES EXCLUDE REGEX "MissionS04\\.cpp$")
list(FILTER SCRIPTS_SOURCES EXCLUDE REGEX "MissionX0\\.cpp$")
list(FILTER SCRIPTS_SOURCES EXCLUDE REGEX "Test_.*\\.cpp$")
list(FILTER SCRIPTS_SOURCES EXCLUDE REGEX "DrMobius\\.cpp$")
list(FILTER SCRIPTS_SOURCES EXCLUDE REGEX "PRDemo\\.cpp$")
list(FILTER SCRIPTS_SOURCES EXCLUDE REGEX "Group\\.cpp$")
list(FILTER SCRIPTS_SOURCES EXCLUDE REGEX "GroupScript\\.cpp$")
list(FILTER SCRIPTS_SOURCES EXCLUDE REGEX "GroupControl\\.cpp$")
list(FILTER SCRIPTS_SOURCES EXCLUDE REGEX "unitcombat\\.cpp$")
# Exclude Toolkit scripts that also rely on default arguments
list(FILTER SCRIPTS_SOURCES EXCLUDE REGEX "Toolkit.*\\.cpp$")

# Build as shared library (DLL)
add_library(scripts SHARED ${SCRIPTS_SOURCES} ${SCRIPTS_HEADERS})

target_include_directories(scripts PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/Code
    ${CMAKE_SOURCE_DIR}/Code/Combat  # Script commands interface
)

# Scripts DLL has limited dependencies
target_link_libraries(scripts PRIVATE
    wwdebug
    wwlib
    wwmath
    winmm  # For timeGetTime
)

# Export macro for DLL
target_compile_definitions(scripts PRIVATE
    SCRIPTS_EXPORTS
    _USRDLL
)

# Output name matching original
set_target_properties(scripts PROPERTIES
    OUTPUT_NAME "Scripts"
    PREFIX ""
)

# Source grouping for IDE
source_group("Source Files" FILES ${SCRIPTS_SOURCES})
source_group("Header Files" FILES ${SCRIPTS_HEADERS})
