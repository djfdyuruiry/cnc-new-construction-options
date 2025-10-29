# NcoRules params
set(RULES_PATH "${CMAKE_SOURCE_DIR}/tiberiandawn/rules")
set(RULES_NCO_PATH "${CMAKE_SOURCE_DIR}/tiberiandawn/rules-nco.cpp")
set(RULE_KEYS_PATH "${CMAKE_SOURCE_DIR}/tiberiandawn/rulekeys-nco.h")
set(RULES_NCO_TEMPLATE_PATH "${RULES_NCO_PATH}.in")
set(RULE_KEYS_TEMPLATE_PATH "${RULE_KEYS_PATH}.in")

# NcoTypes params
set(TYPES_PATH "${CMAKE_SOURCE_DIR}/tiberiandawn/types")
set(TYPES_TEMPLATE_PATH "${CMAKE_SOURCE_DIR}/tiberiandawn/types-nco.cpp.in")

include(NcoRules)
include(NcoTypes)

set(TIBDAWN_SRC
    aadata.cpp
    abstract.cpp
    adata.cpp
    aircraft.cpp
    anim.cpp
    audio.cpp
    base.cpp
    bbdata.cpp
    bdata.cpp
    building.cpp
    bullet.cpp
    cargo.cpp
    ccini.cpp
    cdata.cpp
    cell.cpp
    checkbox.cpp
    cheklist.cpp
    colrlist.cpp
    combat.cpp
    confdlg.cpp
    conquer.cpp
    const.cpp
    coord.cpp
    credits.cpp
    debug.cpp
    dial8.cpp
    dialog.cpp
    display.cpp
    door.cpp
    drive.cpp
    drop.cpp
    edit.cpp
    ending.cpp
    event.cpp
    expand.cpp
    facing.cpp
    factory.cpp
    findpath.cpp
    flasher.cpp
    fly.cpp
    foot.cpp
    fuse.cpp
    gamedlg.cpp
    gauge.cpp
    globals.cpp
    goptions.cpp
    gscreen.cpp
    hdata.cpp
    heap.cpp
    help.cpp
    house.cpp
    iconset.cpp
    idata.cpp
    infantry.cpp
    init.cpp
    intro.cpp
    iomap.cpp
    ioobj.cpp
    jshell.cpp
    layer.cpp
    lib.cpp
    list.cpp
    loaddlg.cpp
    logic.cpp
    map.cpp
    mapeddlg.cpp
    mapedit.cpp
    mapedplc.cpp
    mapedsel.cpp
    mapedtm.cpp
    mapsel.cpp
    menus.cpp
    miscasm.cpp
    mission.cpp
    monoc.cpp
    mouse.cpp
    mplayer.cpp
    msgbox.cpp
    msglist.cpp
    nulldlg.cpp
    object.cpp
    odata.cpp
    options.cpp
    overlay.cpp
    power.cpp
    profile.cpp
    queue.cpp
    radar.cpp
    radio.cpp
    reinf.cpp
    rules.cpp
    saveload.cpp
    scenario.cpp
    scenarioini.cpp
    score.cpp
    scroll.cpp
    sdata.cpp
    shapebtn.cpp
    sidebar.cpp
    slider.cpp
    smudge.cpp
    sounddlg.cpp
    special.cpp
    startup.cpp
    stats.cpp
    super.cpp
    tab.cpp
    tarcom.cpp
    target.cpp
    tdata.cpp
    team.cpp
    teamtype.cpp
    techno.cpp
    template.cpp
    terrain.cpp
    textbtn.cpp
    theme.cpp
    trigger.cpp
    turret.cpp
    txtlabel.cpp
    udata.cpp
    unit.cpp
    visudlg.cpp
    winstub.cpp
    # nco
    typeconverter.cpp
        lua/events/addmessage_luaevent.cpp
        lua/events/addteam_luaevent.cpp
        lua/events/addtrigger_luaevent.cpp
        lua/events/deletetrigger_luaevent.cpp
        lua/events/modifyhousemoney_luaevent.cpp
        lua/events/popup_luaevent.cpp
        lua/game_luaapi.cpp
        lua/messages_luaapi.cpp
        lua/scenario_luaapi.cpp
        lua/scenariolua.cpp
        lua/td_luaapi.cpp
        lua/tdtypes_luaapi.cpp
        lua/ui_luaapi.cpp
)

# Generated code - see: ../cmake/NcoRules.cmake and ./rules
list(APPEND TIBDAWN_SRC
    rules-nco.cpp # RulesClass
)

# Generated code -see: ../cmake/NcoTypes.cmake and ./types
list(APPEND TIBDAWN_SRC
    types-nco.cpp
)

if(NETWORKING)
    list(APPEND TIBDAWN_NET_SRC
        ipxconn.cpp
        ipxgconn.cpp
        ipxmgr.cpp
        netdlg.cpp
    )
endif()

if(CMAKE_CONFIGURATION_TYPES)
    # Glob all the header files together to add to the project for msvc/xcode.
    # Not ideal as CMake won't notice if you add any until something else prompts a CMake run
    # but this is largely a convenience feature for MSVC/XCode users.
    file(GLOB_RECURSE TIBDAWN_HEADERS "*.h")
    set(_TIBDAWN_SOURCES ${TIBDAWN_SRC})

    foreach(_TIBDAWN_SOURCE IN ITEMS ${_TIBDAWN_SOURCES})
        get_filename_component(_TIBDAWN_SOURCE_PATH "${_TIBDAWN_SOURCE}" PATH)
        string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" _TIBDAWN_GROUP_PATH "${_TIBDAWN_SOURCE_PATH}")
        string(REPLACE "/" "\\" _TIBDAWN_GROUP_PATH "${_TIBDAWN_GROUP_PATH}")
        source_group("Source Files\\${_TIBDAWN_GROUP_PATH}" FILES "${_TIBDAWN_SOURCE}")
    endforeach()

    foreach(_TIBDAWN_HEADER IN ITEMS ${TIBDAWN_HEADERS})
        get_filename_component(_TIBDAWN_HEADER_PATH "${_TIBDAWN_HEADER}" PATH)
        string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" _TIBDAWN_HGROUP_PATH "${_TIBDAWN_HEADER_PATH}")
        string(REPLACE "/" "\\" _TIBDAWN_HGROUP_PATH "${_TIBDAWN_HGROUP_PATH}")
        source_group("Header Files\\${_TIBDAWN_HGROUP_PATH}" FILES "${_TIBDAWN_HEADER}")
    endforeach()
endif()

include(ProductVersion)

if(BUILD_REMASTERTD)
    if(WIN32 OR CMAKE_SYSTEM_NAME STREQUAL "Windows")
        # Generate windows version info.
        generate_product_version(
            REMASTERTD_RC
            NAME "Vanilla Conquer TiberianDawn DLL"
            BUNDLE "Vanilla Conquer"
            VERSION_MAJOR 1
            VERSION_MINOR 7
            COMPANY_NAME "Vanilla Conquer"
            COMPANY_COPYRIGHT "Based on code released under GPLv3 by EA"
            ORIGINAL_FILENAME "TiberianDawn.dll"
            RCFILE_PREFIX "RemasterTD"
        )
    endif()
    add_library(TiberianDawn SHARED 
        dllinterface.cpp
        dllinterfaceeditor.cpp
        sidebarglyphx.cpp
        ${TIBDAWN_SRC}
        ${TIBDAWN_HEADERS}
        ${REMASTERTD_RC}
    )
    target_compile_definitions(TiberianDawn PUBLIC $<$<CONFIG:Debug>:_DEBUG> $<$<BOOL:CNC_DEBUG_LOGGING>:DEBUG_LOGGING> ${REMASTER_DEFS} MEGAMAPS)
    target_include_directories(TiberianDawn PUBLIC ${CMAKE_SOURCE_DIR} .)
    target_link_libraries(TiberianDawn commonr ${REMASTER_LIBS} ${STATIC_LIBS})
    set_target_properties(TiberianDawn PROPERTIES PREFIX "")

    # TODO: When visiting remastered support, add lua and relevant data packs (may need to search user paths for lua scripts/rules.ini)
    add_custom_command(TARGET TiberianDawn POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/resources/remaster_mods/Vanilla_TD ${CMAKE_BINARY_DIR}/Vanilla_TD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/Vanilla_TD/Data
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:TiberianDawn> ${CMAKE_BINARY_DIR}/Vanilla_TD/Data/
    )
endif()

if(BUILD_VANILLATD)
    include(BuildIcons)
    # Create the custom command for generating the ico/icns files for display in the OS.
    make_icon(INPUT "${CMAKE_SOURCE_DIR}/resources/vanillatd_icon.svg" OUTPUT VANILLATD_ICON)

    if(WIN32 OR CMAKE_SYSTEM_NAME STREQUAL "Windows")
        # Generate windows version info.
        generate_product_version(
            VANILLATD_RC
            NAME "VanillaTD"
            BUNDLE "Vanilla Conquer"
            VERSION_MAJOR 1
            VERSION_MINOR 0
            COMPANY_NAME "Vanilla Conquer"
            COMPANY_COPYRIGHT "Based on code released under GPLv3 by EA"
            ORIGINAL_FILENAME "vanillatd.exe"
            RCFILE_PREFIX "VanillaTD"
            ICON "${VANILLATD_ICON}"
        )
    endif()

    # A target must include the icon for the custom command to run.
    add_executable(VanillaTD ${TIBDAWN_SRC} ${TIBDAWN_NET_SRC} ${TIBDAWN_HEADERS} ${VANILLATD_RC} ${VANILLATD_ICON})

    if(MAKE_BUNDLE)
        set_source_files_properties(${VANILLATD_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
        set_target_properties(VanillaTD PROPERTIES
            MACOSX_BUNDLE True
            MACOSX_BUNDLE_GUI_IDENTIFIER com.vanilla-conquer.vanillatd
            MACOSX_BUNDLE_BUNDLE_NAME VanillaTD
            MACOSX_BUNDLE_BUNDLE_VERSION "1.0"
            MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
            MACOSX_BUNDLE_COPYRIGHT "Based on code released under GPLv3 by EA"
            MACOSX_BUNDLE_ICON_FILE "vanillatd_icon.icns"
        )
    endif()

    target_compile_definitions(VanillaTD PUBLIC $<$<CONFIG:Debug>:_DEBUG> $<$<BOOL:CNC_DEBUG_LOGGING>:DEBUG_LOGGING> ${VANILLA_DEFS} MEGAMAPS)
    target_include_directories(VanillaTD PUBLIC ${CMAKE_SOURCE_DIR} .)
    target_link_libraries(VanillaTD commonv ${VANILLA_LIBS} ${STATIC_LIBS})
    set_target_properties(VanillaTD PROPERTIES OUTPUT_NAME vanillatd)
    if(MAP_EDITORTD)
        target_compile_definitions(VanillaTD PUBLIC INTERNAL_VERSION)
    endif()
    # Control if we auto generate a console and which "main" function we link using MSVC.
    if(MSVC)
        target_link_options(VanillaTD PRIVATE /subsystem:windows /ENTRY:mainCRTStartup)
    endif()
    if(WIN32 AND NOT MSVC)
        set_target_properties(VanillaTD PROPERTIES LINK_FLAGS "-mwindows")
    endif()
    if(BUILD_WITH_UBSAN)
        target_compile_options(VanillaTD PUBLIC
                    -fsanitize=undefined,float-divide-by-zero,integer,implicit-conversion,implicit-integer-truncation,implicit-integer-arithmetic-value-change,local-bounds,nullability
                    -g3 -fno-omit-frame-pointer)
        target_link_options(VanillaTD PUBLIC -fsanitize=undefined,float-divide-by-zero,integer,implicit-conversion,implicit-integer-truncation,implicit-integer-arithmetic-value-change,local-bounds,nullability)
    endif()
    if(BUILD_WITH_ASAN)
        target_compile_options(VanillaTD PUBLIC
                    -fsanitize=address
                    -g3 -fno-omit-frame-pointer)
        target_link_options(VanillaTD PUBLIC -fsanitize=address)
    endif()

    # create td package: binary + lua library + data packs
    if(MAKE_BUNDLE)
        add_custom_command(
            TARGET VanillaTD POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E rm -rf "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/td"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/td/lua/nco/lib"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/td/lua/nco/TiberianDawn/lib"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/td/vanillatd.app"
            COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different "${CMAKE_SOURCE_DIR}/resources/data/td" "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/td"
            COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different "${CMAKE_SOURCE_DIR}/common/lua/scripts/nco" "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/td/lua/nco"
            COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different "${CMAKE_SOURCE_DIR}/tiberiandawn/lua/scripts/nco" "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/td/lua/nco"
            COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/tools/lua" "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/td/lua"
            COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/vanillatd.app" "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/td/vanillatd.app"
            COMMAND ${CMAKE_COMMAND} -E rm -rf "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/vanillatd.app"
        )
    else()
        set(COPY_TD_BIN_CMD "")

        if(WIN32)
            # include vcpkg built DLLs on Windows
            set(COPY_TD_BIN_CMD COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin" $<TARGET_FILE_DIR:VanillaTD>/td)
        endif()

        add_custom_command(
            TARGET VanillaTD POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E rm -rf $<TARGET_FILE_DIR:VanillaTD>/td
            COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:VanillaTD>/td/lua/nco/lib
            COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:VanillaTD>/td/lua/nco/TiberianDawn/lib
            COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different "${CMAKE_SOURCE_DIR}/resources/data/td" $<TARGET_FILE_DIR:VanillaTD>/td
            COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different "${CMAKE_SOURCE_DIR}/common/lua/scripts/nco" $<TARGET_FILE_DIR:VanillaTD>/td/lua/nco
            COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different "${CMAKE_SOURCE_DIR}/tiberiandawn/lua/scripts/nco" $<TARGET_FILE_DIR:VanillaTD>/td/lua/nco
            COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/tools/lua" $<TARGET_FILE_DIR:VanillaTD>/td/lua
            ${COPY_TD_BIN_CMD}
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:VanillaTD> $<TARGET_FILE_DIR:VanillaTD>/td
            COMMAND ${CMAKE_COMMAND} -E rm -f $<TARGET_FILE:VanillaTD>
        )
    endif()
endif()
