add_subdirectory(pics)
add_subdirectory(sample_kdelnk_files)

include_directories(
    ${KDE1_INCLUDE_DIR}
    )

set(KPANEL_SRCS  kpanel.C props.C layout.C tools.C main.C events.C pmenu.C pfilemenu.C)

set(KPANEL_MOC kpanel.moc pmenu.moc)

QT1_WRAP_MOC(MOC_FILES ${KPANEL_MOC})

add_executable(kpanel ${KPANEL_SRCS} ${MOC_FILES})
target_link_libraries(kpanel
    ${KDE1_KDECORE}
    ${KDE1_KDEUI}
    ${KDE1_KFM}
    Qt::Qt1
    ${X11_LIBRARIES}
    )

install(TARGETS kpanel RUNTIME DESTINATION ${KDE1_BINDIR})

install(FILES personal.kdelnk DESTINATION ${KDE1_DATADIR}/kpanel/default RENAME personal_directory)
install(FILES Home.kdelnk Desktop.kdelnk DESTINATION ${KDE1_DATADIR}/kpanel/default)
