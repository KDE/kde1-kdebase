include_directories(
    ${QT_INCLUDE_DIR}
    ${KDE1_INCLUDE_DIR}
    )

set(SYSSOUND_SRCS
    main.cpp bell.cpp kslidercontrol.cpp
)

set(SYSSOUND_MOC
    bell.moc kslidercontrol.moc
)
QT1_WRAP_MOC(MOC_FILES ${SYSSOUND_MOC})

add_executable(kcmbell ${SYSSOUND_SRCS} ${MOC_FILES})
target_link_libraries(kcmbell
    ${QT_LIBRARIES}
    ${X11_LIBRARIES}
    ${KDE1_KDECORE}
    ${KDE1_KDEUI}
    )

install(TARGETS kcmbell RUNTIME DESTINATION ${KDE1_BINDIR})

install_icon(bell.xpm "")
