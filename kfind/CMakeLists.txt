add_subdirectory(pics)

include_directories(
    ${KDE1_INCLUDE_DIR}
    )

set(KFIND_SRCS
    kfarch.cpp kfdird.cpp kfindtop.cpp  kftabdlg.cpp
    kfsave.cpp kftypes.cpp kfoptions.cpp kfwin.cpp kfind.cpp
    main.cpp moc.cpp)

set(KFIND_MOC kfdird.moc kfwin.moc kfindtop.moc
    kftabdlg.moc kfoptions.moc kfind.moc)

QT1_WRAP_MOC(MOC_FILES ${KFIND_MOC})

add_executable(kfind ${KFIND_SRCS} ${MOC_FILES})
target_link_libraries(kfind
    ${KDE1_KDECORE}
    ${KDE1_KDEUI}
    ${KDE1_KFM}
    ${KDE1_KFILE}
    Qt::Qt1
    ${X11_LIBRARIES}
    )

install(TARGETS kfind RUNTIME DESTINATION ${KDE1_BINDIR})

install_icon(kfind.xpm "")
