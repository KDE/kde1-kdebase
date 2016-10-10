add_subdirectory(pics)

include_directories(
    ${QT_INCLUDE_DIR}
    ${KDE1_INCLUDE_DIR}
    )

set(KPANEL_SRCS  kpanel.C props.C layout.C tools.C main.C events.C pmenu.C pfilemenu.C)

set(KPANEL_MOC kpanel.moc pmenu.moc)

QT1_WRAP_MOC(MOC_FILES ${KPANEL_MOC})

add_executable(kpanel ${KPANEL_SRCS} ${MOC_FILES})
target_link_libraries(kpanel
    ${QT_LIBRARIES}
    ${X11_LIBRARIES}
    ${KDE1_KDECORE}
    ${KDE1_KDEUI}
    ${KDE1_KFM}
    )

install(TARGETS kpanel RUNTIME DESTINATION ${KDE1_BINDIR})

install(FILES personal.kdelnk DESTINATION ${KDE1_DATADIR}/kpanel/default RENAME personal_directory)
install(FILES Home.kdelnk Desktop.kdelnk DESTINATION ${KDE1_DATADIR}/kpanel/default)
