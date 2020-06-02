include_directories(
    ${KDE1_INCLUDE_DIR}
    )

set(KWM_SRCS
    main.cpp bell.cpp kslidercontrol.cpp
)

set(KWM_MOC
    bell.moc kslidercontrol.moc
)
QT1_WRAP_MOC(MOC_FILES ${KWM_MOC})

add_executable(kcmbell ${KWM_SRCS} ${MOC_FILES})
target_link_libraries(kcmbell
    Qt::Qt1
    ${X11_LIBRARIES}
    ${KDE1_KDECORE}
    ${KDE1_KDEUI}
    )

install(TARGETS kcmbell RUNTIME DESTINATION ${KDE1_BINDIR})

install_icon(kwm "")
