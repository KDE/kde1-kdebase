include_directories(
    ${KDE1_INCLUDE_DIR}
    )

set(SAMBA_SRCS
    main.cpp bell.cpp kslidercontrol.cpp
)

set(SAMBA_MOC
    bell.moc kslidercontrol.moc
)
QT1_WRAP_MOC(MOC_FILES ${SAMBA_MOC})

add_executable(kcmbell ${SAMBA_SRCS} ${MOC_FILES})
target_link_libraries(kcmbell
    Qt::Qt1
    ${X11_LIBRARIES}
    ${KDE1_KDECORE}
    ${KDE1_KDEUI}
    )

install(TARGETS kcmbell RUNTIME DESTINATION ${KDE1_BINDIR})

install_icon(samba "")
