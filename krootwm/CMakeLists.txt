include_directories(
    ${KDE1_INCLUDE_DIR}
    )

set(KROOTWM_SRCS krootwm.C)

set(KROOTWM_MOC krootwm.moc)

QT1_WRAP_MOC(MOC_FILES ${KROOTWM_MOC})

add_executable(krootwm ${KROOTWM_SRCS} ${MOC_FILES})
target_link_libraries(krootwm
    ${KDE1_KDECORE}
    ${KDE1_KDEUI}
    ${KDE1_KFM}
    Qt::Qt1
    ${X11_LIBRARIES}
    )

install(TARGETS krootwm RUNTIME DESTINATION ${KDE1_BINDIR})
