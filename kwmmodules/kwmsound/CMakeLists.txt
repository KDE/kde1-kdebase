include_directories(
    ${KDE1_INCLUDE_DIR}
    )

set(KWMSOUND_SRCS kwmsound.C)

set(KWMSOUND_MOC kwmsound.moc)

QT1_WRAP_MOC(MOC_FILES ${KWMSOUND_MOC})

add_executable(kwmsound ${KWMSOUND_SRCS} ${MOC_FILES})
target_link_libraries(kwmsound
    Qt::Qt1
    ${X11_LIBRARIES}
    ${KDE1_KDECORE}
    ${KDE1_KDEUI}
    ${KDE1_MEDIATOOL}
    )

install(TARGETS kwmsound RUNTIME DESTINATION ${KDE1_BINDIR})
