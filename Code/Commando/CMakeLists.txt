# Commando - Main game executable
# WinMain entry point, networking, multiplayer modes, dialogs, game modes

file(GLOB COMMANDO_SOURCES "*.cpp" "*.c")
file(GLOB COMMANDO_HEADERS "*.h")

# Resource files
file(GLOB COMMANDO_RESOURCES "*.rc")

# Include WWOnline sources (compiled into Commando)
file(GLOB WWONLINE_SOURCES "${CMAKE_SOURCE_DIR}/Code/WWOnline/*.cpp")
file(GLOB WWONLINE_HEADERS "${CMAKE_SOURCE_DIR}/Code/WWOnline/*.h")

# Include wolapi interface files
file(GLOB WOLAPI_SOURCES "${CMAKE_SOURCE_DIR}/Code/wolapi/*.c")
file(GLOB WOLAPI_HEADERS "${CMAKE_SOURCE_DIR}/Code/wolapi/*.h")

# Include WOLBrowser interface files
file(GLOB WOLBROWSER_SOURCES "${CMAKE_SOURCE_DIR}/Code/WOLBrowser/*.c")
file(GLOB WOLBROWSER_HEADERS "${CMAKE_SOURCE_DIR}/Code/WOLBrowser/*.h")

add_executable(renegade WIN32
    ${COMMANDO_SOURCES}
    ${COMMANDO_HEADERS}
    ${COMMANDO_RESOURCES}
    ${WWONLINE_SOURCES}
    ${WWONLINE_HEADERS}
    ${WOLAPI_SOURCES}
    ${WOLAPI_HEADERS}
    ${WOLBROWSER_SOURCES}
    ${WOLBROWSER_HEADERS}
)

target_include_directories(renegade PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/Code
    ${CMAKE_SOURCE_DIR}/Code/Combat
    ${CMAKE_SOURCE_DIR}/Code/WWOnline
    ${CMAKE_SOURCE_DIR}/Code/wolapi
    ${CMAKE_SOURCE_DIR}/Code/WOLBrowser
    ${CMAKE_SOURCE_DIR}/Code/DirectX/include
    ${CMAKE_SOURCE_DIR}/Code/Miles6/include
    ${CMAKE_SOURCE_DIR}/Code/GameSpy
)

target_link_libraries(renegade PRIVATE
    # Engine libraries
    wwdebug
    wwlib
    wwmath
    ww3d2
    wwphys
    wwnet
    wwsaveload
    wwtranslatedb
    wwbitpack
    wwui
    wwaudio
    wwutil

    # Game libraries
    combat
    binkmovie
    bandtest
    scontrol
    gamespy

    # Windows libraries
    user32
    gdi32
    winspool
    comdlg32
    advapi32
    ole32
    oleaut32
    uuid
    winmm
    shell32
    version
    wsock32
    ws2_32
)

# DirectX libraries
target_link_libraries(renegade PRIVATE
    d3d9
    d3dx9
    dxguid
    dsound
    dinput8
)

target_compile_definitions(renegade PRIVATE
    DIRECTX
)

# Output names for different configurations
set_target_properties(renegade PROPERTIES
    OUTPUT_NAME_DEBUG "RenegadeD"
    OUTPUT_NAME_RELEASE "Renegade"
    OUTPUT_NAME_PROFILE "RenegadeP"
)

# Generate map files and embed manifest for UAC compatibility
if(MSVC)
    set_target_properties(renegade PROPERTIES
        LINK_FLAGS "/MAP /MANIFEST:EMBED /MANIFESTINPUT:\"${CMAKE_SOURCE_DIR}/manifests/Renegade.exe.manifest\""
    )
endif()

# Source grouping for IDE
source_group("Combat" FILES ${COMMANDO_SOURCES} ${COMMANDO_HEADERS})
source_group("WWOnline" FILES ${WWONLINE_SOURCES} ${WWONLINE_HEADERS})
source_group("wolapi" FILES ${WOLAPI_SOURCES} ${WOLAPI_HEADERS})
source_group("WOLBrowser" FILES ${WOLBROWSER_SOURCES} ${WOLBROWSER_HEADERS})
source_group("Resources" FILES ${COMMANDO_RESOURCES})
