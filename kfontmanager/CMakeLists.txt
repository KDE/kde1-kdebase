include_directories(
    ${KDE1_INCLUDE_DIR}
    )

set(KFONTMANAGER_SRCS
    main.cpp kfontmanager.cpp kfontlist.cpp kfontexplorer.cpp)
set(KFONTMANAGER_MOC kfontmanager.moc kfontlist.moc kfontexplorer.moc)

QT1_WRAP_MOC(MOC_FILES ${KFONTMANAGER_MOC})

add_executable(kfontmanager1 ${KFONTMANAGER_SRCS} ${MOC_FILES})
target_link_libraries(kfontmanager1
    ${KDE1_KDECORE}
    Qt::Qt1
    ${X11_LIBRARIES}
    m
    )

install(TARGETS kfontmanager1 RUNTIME DESTINATION ${KDE1_BINDIR})

install_icon(kfontmanager.xpm "")
