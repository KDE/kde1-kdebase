include_directories(
    ${KDE1_INCLUDE_DIR}
    )

set(SYSSOUND_SRCS
    main.cpp syssound.cpp
)

set(SYSSOUND_MOC
    syssound.moc
)

set(SOUNDFILES
    sounds/KDE_Close_Window.wav
    sounds/KDE_Window_Iconify.wav
    sounds/KDE_Window_Sticky.wav
    sounds/KDE_Logout.wav
    sounds/KDE_Window_Maximize.wav
    sounds/KDE_Window_UnMaximize.wav
    sounds/KDE_Startup.wav
    sounds/KDE_Window_Shade_Down.wav
    sounds/KDE_Window_UnSticky.wav
    sounds/KDE_Window_DeIconify.wav
    sounds/KDE_Window_Shade_Up.wav
)

QT1_WRAP_MOC(MOC_FILES ${SYSSOUND_MOC})

add_executable(kcmsyssound ${SYSSOUND_SRCS} ${MOC_FILES})
target_link_libraries(kcmsyssound
    Qt::Qt1
    ${X11_LIBRARIES}
    ${KDE1_KDECORE}
    ${KDE1_KDEUI}
    ${KDE1_MEDIATOOL}
    )

install(TARGETS kcmsyssound RUNTIME DESTINATION ${KDE1_BINDIR})

install(FILES ${SOUNDFILES} DESTINATION ${KDE1_SOUNDDIR})

install_icon(kcmsyssound.xpm "")
