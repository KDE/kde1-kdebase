include_directories(
    ${QT_INCLUDE_DIR}
    ${KDE1_INCLUDE_DIR}
    )

set(KFMCLINET_SRCS kfmclient.cpp ../debug.cpp)

add_executable(kfmclient ${KFMCLINET_SRCS} ${MOC_FILES})
target_link_libraries(kfmclient
    ${QT_LIBRARIES}
    ${X11_LIBRARIES}
    ${KDE1_KDECORE}
    ${KDE1_KDEUI}
    ${KDE1_KFM}
    ${KDE1_KFILE}
    )

install(TARGETS kfmclient RUNTIME DESTINATION ${KDE1_BINDIR})
