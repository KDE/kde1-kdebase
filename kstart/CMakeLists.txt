include_directories(
    ${QT_INCLUDE_DIR}
    ${KDE1_INCLUDE_DIR}
    )

set(KSTART_SRCS kstart.C)

set(KSTART_MOC kstart.moc)

QT1_WRAP_MOC(MOC_FILES ${KSTART_MOC})

add_executable(kstart ${KSTART_SRCS} ${MOC_FILES})
target_link_libraries(kstart
    ${QT_LIBRARIES}
    ${X11_LIBRARIES}
    ${KDE1_KDECORE}
    ${KDE1_KDEUI}
    )

install(TARGETS kstart RUNTIME DESTINATION ${KDE1_BINDIR})

