include_directories(
    ${KDE1_INCLUDE_DIR}
    )

set(KWMCCOM_SRCS kwmcom.C)

QT1_WRAP_MOC(MOC_FILES ${KWMCCOM_MOC})

add_executable(kwmcom ${KWMCCOM_SRCS} ${MOC_FILES})
target_link_libraries(kwmcom
    ${KDE1_KDECORE}
    Qt::Qt1
    ${X11_LIBRARIES}
    )

install(TARGETS kwmcom RUNTIME DESTINATION ${KDE1_BINDIR})
