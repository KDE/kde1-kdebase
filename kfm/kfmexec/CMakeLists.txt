include_directories(
    ${QT_INCLUDE_DIR}
    ${KDE1_INCLUDE_DIR}
    )

set(KFMWARN_SRCS main.cpp)

QT1_WRAP_MOC(MOC_FILES main.moc)

add_executable(kfmexec ${KFMWARN_SRCS} ${MOC_FILES})
target_link_libraries(kfmexec
    ${QT_LIBRARIES}
    ${X11_LIBRARIES}
    ${KDE1_KDECORE}
    ${KDE1_KFM}
    )

install(TARGETS kfmexec RUNTIME DESTINATION ${KDE1_BINDIR})

