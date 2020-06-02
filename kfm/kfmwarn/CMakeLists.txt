include_directories(
    ${KDE1_INCLUDE_DIR}
    )

set(KFMWARN_SRCS main.cpp)

add_executable(kfmwarn ${KFMWARN_SRCS})
target_link_libraries(kfmwarn
    ${X11_LIBRARIES}
    ${KDE1_KDECORE}
    Qt::Qt1
    )

install(TARGETS kfmwarn RUNTIME DESTINATION ${KDE1_BINDIR})

