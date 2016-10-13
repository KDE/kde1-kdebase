function (install_themes)
    set(list_var "${ARGN}")
    foreach (it IN LISTS list_var)
        get_filename_component(theme ${it} NAME_WE)
        add_custom_command(
            OUTPUT ${theme}.tar.gz
            COMMAND tar
            ARGS cfz ${CMAKE_CURRENT_BINARY_DIR}/${theme}.tar.gz ${theme}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            )
        list(APPEND elements ${theme}.tar.gz)
        install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${theme}.tar.gz"
            DESTINATION ${KDE1_DATADIR}/kthememgr/Themes)
    endforeach ()
    add_custom_target(allthemes ALL DEPENDS ${elements})
endfunction ()

install(FILES Template.themerc DESTINATION ${KDE1_DATADIR}/kthememgr/Themes)

install_themes(Astronique Bryce Default Glax MGBreizh Nostalgy Wood)
