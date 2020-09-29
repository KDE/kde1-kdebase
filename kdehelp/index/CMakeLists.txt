include_directories(
    ${KDE1_INCLUDE_DIR}
    )

add_executable(helpindex kdeindex.cpp)
target_link_libraries(helpindex
    ${KDE1_KDECORE}
    Qt::Qt1
    )

install(TARGETS helpindex RUNTIME DESTINATION ${KDE1_CGIDIR})
