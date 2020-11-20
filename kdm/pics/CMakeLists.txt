install(FILES
    kdelogo.xpm kdeflower.xpm session_kde.xpm
    DESTINATION ${KDE1_DATADIR}/kdm/pics
    )

install(FILES
    default.xpm default2.xpm root.xpm root2.xpm
    DESTINATION ${KDE1_DATADIR}/kdm/pics/users
    )

install(FILES
    kdm_bg.jpg
    DESTINATION ${KDE1_WALLPAPERDIR}
    )
