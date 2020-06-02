add_subdirectory(pics)

include_directories(
    ${KDE1_INCLUDE_DIR}
    )

set(INFO_SRCS
    main.cpp memory.cpp
)

set(INFO_MOC
    memory.moc info.moc
)
QT1_WRAP_MOC(MOC_FILES ${INFO_MOC})

add_executable(kcminfo ${INFO_SRCS} ${MOC_FILES})
target_link_libraries(kcminfo
    Qt::Qt1
    ${X11_LIBRARIES}
    ${KDE1_KDECORE}
    ${KDE1_KDEUI}
    )

install(TARGETS kcminfo RUNTIME DESTINATION ${KDE1_BINDIR})
