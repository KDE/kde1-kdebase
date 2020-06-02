include_directories(
    ${KDE1_INCLUDE_DIR}
    )

set(KEYS_SRCS
main.cpp keyconfig.cpp savescm.cpp
)

set(KEYS_MOC
keyconfig.moc savescm.moc  
)
QT1_WRAP_MOC(MOC_FILES ${KEYS_MOC})

add_executable(kcmkeys ${KEYS_SRCS} ${MOC_FILES})
target_link_libraries(kcmkeys
    Qt::Qt1
    ${X11_LIBRARIES}
    ${KDE1_KDECORE}
    ${KDE1_KDEUI}
    )

install(TARGETS kcmkeys RUNTIME DESTINATION ${KDE1_BINDIR})
