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
target_link_libraries(kwm ${QT_LIBRARIES} ${X11_LIBRARIES} ${LIB_KDECORE} ${LIB_KFM} ${LIB_KDEUI})

install(TARGETS kwm RUNTIME DESTINATION bin)

