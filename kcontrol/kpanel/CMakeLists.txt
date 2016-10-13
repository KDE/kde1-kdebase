include_directories(
    ${QT_INCLUDE_DIR}
    ${KDE1_INCLUDE_DIR}
    )

set(KPANEL_SRCS
main.cpp panel.cpp desktops.cpp options.cpp disknav.cpp
)

set(KPANEL_MOC
panel.moc desktops.moc options.moc disknav.moc
)
QT1_WRAP_MOC(MOC_FILES ${KPANEL_MOC})

add_executable(kcmkpanel ${KPANEL_SRCS} ${MOC_FILES})
target_link_libraries(kcmkpanel
    ${QT_LIBRARIES}
    ${X11_LIBRARIES}
    ${KDE1_KDECORE}
    ${KDE1_KDEUI}
    )

install(TARGETS kcmkpanel RUNTIME DESTINATION ${KDE1_BINDIR})

install_icon(kcmkpanel.xpm "")
