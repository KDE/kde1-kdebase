include_directories(
    ${QT_INCLUDE_DIR}
    ${KDE1_INCLUDE_DIR}
    )

add_executable(krdb krdb.cpp)
target_link_libraries(kfind
    ${QT_LIBRARIES}
    ${X11_LIBRARIES}
    ${KDE1_KDECORE}
    ${KDE1_KDEUI}
    )

install(TARGETS krdb RUNTIME DESTINATION ${KDE1_BINDIR})
install(FILES
AAAAAAGeneral.ad AAAMotif.ad AAATk.ad AAAXaw.ad Editres.ad
	Emacs.ad ML.ad Nedit.ad Netscape.ad XCalc.ad XTerm.ad XV.ad
	XWp.ad RVPlayer.ad 
    DESTINATION ${KDE1_DATADIR}/kdisplay/app-defaults)
