include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${KDE1_INCLUDE_DIR}
    )

add_subdirectory(pics)
set(KCONTROL_SRCS
configlist.cpp main.cpp toplevel.cpp kdelnk.cpp mainwidget.cpp kswallow.cpp
)
set(KCONTROL_MOC toplevel.moc kdelnk.moc mainwidget.moc configlist.moc kswallow.moc)

QT1_WRAP_MOC(MOC_FILES ${KCONTROL_MOC})

add_executable(kcontrol ${KCONTROL_SRCS} ${MOC_FILES})
target_link_libraries(kcontrol
    Qt::Qt1
    ${X11_LIBRARIES}
    ${KDE1_KDECORE}
    ${KDE1_KDEUI}
    )

install(TARGETS kcontrol RUNTIME DESTINATION ${KDE1_BINDIR})

install_icon(kcontrol.xpm "")
