include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${QT_INCLUDE_DIR}
    ${KDE1_INCLUDE_DIR}
    )

add_subdirectory(kfmwarn)
add_subdirectory(client)
add_subdirectory(kioslave)
add_subdirectory(pics)
add_subdirectory(Desktop)
add_subdirectory(kfmexec)
add_subdirectory(config)

set(KFM_SRCS
	kfmgui.cpp kfmview.cpp kbind.cpp main.cpp
	kfmdlg.cpp bookmark.cpp kfmprops.cpp kfmserver_ipc.cpp
	kfmserver_ipc2.cpp kfmserver.cpp kioserver.cpp kioserver_ipc.cpp
	kioserver_ipc2.cpp kfmipc.cpp root.cpp kfmman.cpp kiojob.cpp
	htmlcache.cpp autostart.cpp kfmtree.cpp
	krenamewin.cpp passworddialog.cpp kURLcompletion.cpp debug.cpp kfmw.cpp
	kfmpaths.cpp kfmjob.cpp kfmexec.cpp kmimemagic.cpp kfinder.cpp utils.cpp
	open-with.cpp finddlg.cpp kcookiejar.cpp kcookiewin.cpp popup.cpp kintlist.cpp)

set(KFM_MOC
	kfmgui.h kfmview.h kbind.h
	kfmdlg.h bookmark.h kfmprops.h kfmserver_ipc.h
	kfmserver.h kioserver.h kioserver_ipc.h
	root.h kfmman.h kiojob.h
	htmlcache.h kfmtree.h
	krenamewin.h passworddialog.h kURLcompletion.h kfmw.h
	kfmjob.h kfmexec.h kfinder.h
	open-with.h finddlg.h kcookiewin.h popup.h)


QT1_WRAP_HEADER(MOC_FILES ${KFM_MOC})

add_executable(kfm ${KFM_SRCS} ${MOC_FILES})
target_link_libraries(kfm
    ${QT_LIBRARIES}
    ${X11_LIBRARIES}
    ${KDE1_KDECORE}
    ${KDE1_KDEUI}
    ${KDE1_KHTMLW}
    ${KDE1_JSCRIPT}
    ${KDE1_KFM}
    ${KDE1_KIMGIO}
    ${KDE1_KFILE}
    )

install(TARGETS kfm RUNTIME DESTINATION ${KDE1_BINDIR})

install(FILES directory.trash directory.autostart directory.templates directory.desktop DESTINATION ${KDE1_DATADIR}/kfm)

install(FILES kfmsu kfmsu2 startkde kde unpack
    PERMISSIONS WORLD_EXECUTE WORLD_READ
    DESTINATION ${KDE1_BINDIR})

install(FILES config/kfmrc config/desktop DESTINATION ${KDE1_CONFDIR})
install(FILES magic DESTINATION ${KDE1_MIMEDIR})

install_icon(kfm.xpm "")
install_icon(link.xpm "")
