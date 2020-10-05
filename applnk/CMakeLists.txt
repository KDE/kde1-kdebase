install(FILES
    Help.kdelnk Home.kdelnk Kfind.kdelnk
    Krefresh.kdelnk Trash.kdelnk
    KControl.kdelnk .directory
    DESTINATION ${KDE1_APPSDIR}
)

install(DIRECTORY Applications Games Graphics Internet Multimedia System Utilities Settings
    DESTINATION ${KDE1_APPSDIR}
    PATTERN Makefile.* EXCLUDE
    PATTERN *.xpm EXCLUDE
)

install(FILES
    Settings/Information/network.xpm
    Settings/Information/information.xpm
    DESTINATION ${KDE1_ICONDIR}
)
