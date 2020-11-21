configure_file(xdm-conf.in xdm-config)
configure_file(Xserv.in Xservers)
install(FILES
    Xresources Xservers Xaccess Xsession
    GiveConsole TakeConsole Xsetup_0
    ${CMAKE_CURRENT_BINARY_DIR}/xdm-config
    DESTINATION ${KDE1_XDMDIR}
    )

