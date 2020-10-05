add_subdirectory(pics)
include_directories(
    ${KDE1_INCLUDE_DIR}
    )

set(KCMKWM_SRCS
    main.cpp
    titlebar.cpp
    windows.cpp
    desktop.cpp
    mouse.cpp
    advanced.cpp
)

set(KCMKWM_MOC
    advanced.moc
    desktop.moc
    mouse.moc
    titlebar.moc
    windows.moc
)
QT1_WRAP_MOC(MOC_FILES ${KCMKWM_MOC})

add_executable(kcmkwm ${KCMKWM_SRCS} ${MOC_FILES})
target_link_libraries(kcmkwm
    Qt::Qt1
    ${KDE1_KDECORE}
    ${KDE1_KDEUI}
    )

install(TARGETS kcmkwm RUNTIME DESTINATION ${KDE1_BINDIR})

install_icon(kwm "")
