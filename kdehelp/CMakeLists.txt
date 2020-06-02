include_directories(
    ${KDE1_INCLUDE_DIR}
    )

set(KDEHELP_SRCS main.cpp helpwin.cpp info.cpp error.cpp misc.cpp
    options.cpp man.cpp bookmark.cpp cgi.cpp mainwidget.cpp
    history.cpp booktoken.cpp finddlg.cpp)

set(KDEHELP_MOC bookmark.moc cgi.moc finddlg.moc.moc helpwin.moc mainwidget.moc options.moc)

QT1_WRAP_CPP(MOC_FILES ${KDEHELP_MOC})

add_executable(kdehelp ${KDEHELP_SRCS} ${MOC_FILES})
target_link_libraries(kdehelp
    ${KDE1_KDECORE}
    ${KDE1_KHTMLW}
    ${KDE1_KFILE}
    ${KDE1_KIMGIO}
    ${KDE1_KDEUI}
    ${KDE1_KFM}
    ${KDE1_JSCRIPT}
    Qt::Qt1
    ${X11_LIBRARIES}
    )

install(TARGETS kdehelp RUNTIME DESTINATION ${KDE1_BINDIR})

install(FILES mini-kdehelp.xpm DESTINATION ${KDE1_MINIDIR} RENAME kdehelp.xpm)
install(FILES large-kdehelp.xpm DESTINATION ${KDE1_ICONDIR}/large RENAME kdehelp.xpm)
