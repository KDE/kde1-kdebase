include_directories(
    ${KDE1_INCLUDE_DIR}
    )

set(KFMWARN_SRCS main.cpp)

QT1_WRAP_MOC(MOC_FILES main.moc)

add_executable(kfmexec ${KFMWARN_SRCS} ${MOC_FILES})
target_link_libraries(kfmexec
    ${KDE1_KDECORE}
    ${KDE1_KFM}
    Qt::Qt1
    ${X11_LIBRARIES}
    )

install(TARGETS kfmexec RUNTIME DESTINATION ${KDE1_BINDIR})

