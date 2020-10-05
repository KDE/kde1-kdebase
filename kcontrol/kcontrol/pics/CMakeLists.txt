INSTALL(FILES
    application_settings.xpm
    desktop_settings.xpm
    general_settings.xpm
    information_settings.xpm
    input_devices_settings.xpm
    network_settings.xpm
    panel_settings.xpm
    sound_settings.xpm
    kcontrol.xpm
    DESTINATION ${KDE1_ICONDIR}
)

INSTALL(FILES
    kdekcc.xpm
    DESTINATION ${KDE1_ICONDIR}/kcontrol/pics/
)

# Have to install mini/large manually because they're in separate dirs
INSTALL(FILES
    mini/application_settings.xpm
    mini/desktop_settings.xpm
    mini/general_settings.xpm
    mini/information_settings.xpm
    mini/input_devices_settings.xpm
    mini/network_settings.xpm
    mini/panel_settings.xpm
    mini/sound_settings.xpm
    mini/kcontrol.xpm
    DESTINATION ${KDE1_ICONDIR}/mini/
)

INSTALL(FILES
    large/desktop_settings.xpm
    DESTINATION ${KDE1_ICONDIR}/large/
)
