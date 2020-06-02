include_directories(
    ${KDE1_INCLUDE_DIR}
    )

set(KPAGER_SRCS
windowdrag.cpp ktrianglebutton.cpp desktop.cpp kpagerclient.cpp kpager.cpp main.cpp
)

set(KPAGER_MOC
desktop.h kpager.h kpagerclient.h ktrianglebutton.h version.h windowdrag.h
)

QT1_WRAP_CPP(MOC_FILES ${KPAGER_MOC})

add_executable(kpager ${KPAGER_SRCS} ${MOC_FILES})
target_link_libraries(kpager
    Qt::Qt1
    ${X11_LIBRARIES}
    ${KDE1_KDECORE}
    ${KDE1_KDEUI}
    ${KDE1_KIMGIO}
    ${KDE1_KFILE}
    ${KDE1_KFM}
    )

install(TARGETS kpager RUNTIME DESTINATION ${KDE1_BINDIR})
install(FILES kpager.kdelnk DESTINATION ${KDE1_APPSDIR}/System)
