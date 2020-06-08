include_directories(
    ${KDE1_INCLUDE_DIR}
    )

set(KFMCLINET_SRCS kfmclient.cpp ../debug.cpp)

add_executable(kfmclient1 ${KFMCLINET_SRCS} ${MOC_FILES})
target_link_libraries(kfmclient1
    Qt::Qt1
    ${X11_LIBRARIES}
    ${KDE1_KDECORE}
    ${KDE1_KDEUI}
    ${KDE1_KFM}
    ${KDE1_KFILE}
    )

install(TARGETS kfmclient1 RUNTIME DESTINATION ${KDE1_BINDIR})
