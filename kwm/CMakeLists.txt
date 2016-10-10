#add_subdirectory(pics)

include_directories(
    ${QT_INCLUDE_DIR}
    ${KDE1_INCLUDE_DIR}
    )

set(KWM_SRCS client.C manager.C main.C minicli.C warning.C drag.C
    logout.C taskmgr.C kURLcompletion.cpp gradientFill.C )

set( METASOURCES client.moc manager.moc main.moc minicli.moc
    warning.moc logout.moc taskmgr.moc kURLcompletion.moc )

QT1_WRAP_MOC(MOC_SRCS ${METASOURCES})

add_executable(kwm ${KWM_SRCS} ${MOC_SRCS})
target_link_libraries(kwm 
    ${QT_LIBRARIES} 
    ${X11_LIBRARIES}
    ${KDE1_KDECORE}
    ${KDE1_KHTMLW}
    ${KDE1_KFILE}
    ${KDE1_KIMGIO}
    ${KDE1_KDEUI}
    ${KDE1_KFM}
    ${KDE1_JSCRIPT}
    )

install(TARGETS kwm RUNTIME DESTINATION ${KDE1_BINDIR})

