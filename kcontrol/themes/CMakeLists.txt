add_subdirectory(Themes)

include_directories(
    ${QT_INCLUDE_DIR}
    ${KDE1_INCLUDE_DIR}
    )

set(THEMES_SRCS
theme.cpp installer.cpp main.cpp kwmcom.cpp options.cpp widgetcanvas.cpp about.cpp groupdetails.cpp themecreator.cpp newthemedlg.cpp
)

set(THEMES_MOC
theme.moc installer.moc colors.moc widgetcanvas.moc options.moc about.moc groupdetails.moc themecreator.moc
)
QT1_WRAP_MOC(MOC_FILES ${THEMES_MOC})

add_executable(kthememgr ${THEMES_SRCS} ${MOC_FILES})
target_link_libraries(kthememgr
    ${QT_LIBRARIES}
    ${X11_LIBRARIES}
    ${KDE1_KDECORE}
    ${KDE1_KDEUI}
    ${KDE1_KFILE}
    ${KDE1_KFM}
    ${KDE1_KIMGIO}
    )

install(TARGETS kthememgr RUNTIME DESTINATION ${KDE1_BINDIR})

install_icon(kthememgr.xpm "")
install(FILES kthememgr.kdelnk DESTINATION ${KDE1_APPSDIR}/Settings/Desktop)
install(FILES theme.mappings DESTINATION ${KDE1_DATADIR}/kthememgr)
