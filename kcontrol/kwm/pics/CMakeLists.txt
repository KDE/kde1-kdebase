INSTALL(FILES
    buttons.xpm kcmkwm.xpm properties.xpm titlebar.xpm
    DESTINATION ${KDE1_ICONDIR}
)

# Have to install mini/large manually because they're in separate dirs
INSTALL(FILES
    mini/buttons.xpm
    mini/kcmkwm.xpm
    mini/titlebar.xpm
    mini/windows.xpm
    mini/winprops.xpm
    DESTINATION ${KDE1_ICONDIR}/mini/
)

INSTALL(FILES
    large/buttons.xpm
    large/titlebar.xpm
    large/properties.xpm
    DESTINATION ${KDE1_ICONDIR}/large/
)
