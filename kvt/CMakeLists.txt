include_directories(
    ${KDE1_INCLUDE_DIR}
    ${CURSES_INCLUDE_DIRS}
    )

set(KVT_SRCS command.c screen.c utmp.c
	xsetup.c debug.c resources.c main.C
	grkelot.c)

set(KVT_MOC main.moc)

QT1_WRAP_MOC(MOC_FILES ${KVT_MOC})

add_executable(kvt ${KVT_SRCS} ${MOC_FILES})
target_link_libraries(kvt
    ${KDE1_KDECORE}
    ${KDE1_KDEUI}
    Qt::Qt1
    ${X11_LIBRARIES}
    ${CURSES_LIBRARIES}
    )

install(TARGETS kvt RUNTIME DESTINATION ${KDE1_BINDIR})

install_icon(kvt.xpm "")
