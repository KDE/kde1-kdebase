include_directories(
    ${QT_INCLUDE_DIR}
    ${KDE1_INCLUDE_DIR}
    )

set(INPUT_SRCS
main.cpp mouse.cpp keyboard.cpp kslidercontrol.cpp
)

set(INPUT_MOC
mouse.moc keyboard.moc kslidercontrol.moc
)
QT1_WRAP_MOC(MOC_FILES ${INPUT_MOC})

add_executable(kcminput ${INPUT_SRCS} ${MOC_FILES})
target_link_libraries(kcminput
    ${QT_LIBRARIES}
    ${X11_LIBRARIES}
    ${KDE1_KDECORE}
    ${KDE1_KDEUI}
    )

install(TARGETS kcminput RUNTIME DESTINATION ${KDE1_BINDIR})

install_icon(mouse.xpm "")
install_icon(keyboard.xpm "")
