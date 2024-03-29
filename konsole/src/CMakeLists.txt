include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${KDE1_INCLUDE_DIR}
    )


#install(FILES kimgio.h DESTINATION ${KDE1_INCLUDES})

set(KONSOLE_SRCS main.C schema.C session.C)

set(KONSOLE_MOC
    TEWidget.moc
    TEmuVt102.moc
    session.moc
    main.moc
    TEmulation.moc
    TEShell.moc)

QT1_WRAP_HEADER(MOC_FILES ${KONSOLE_MOC})

set(LIBKONSOLE_SRCS TEWidget.C TEmuVt102.C TEScreen.C TEmulation.C TEShell.C)
add_library(libkonsole STATIC ${LIBKONSOLE_SRCS} ${MOC_FILES})
target_link_libraries(libkonsole Qt::Qt1)
set_target_properties(libkonsole PROPERTIES CMAKE_EXE_LINKER_FLAGS "-version-info 9:1.9")
#set_target_properties(konsole PROPERTIES VERSION 0.1.9 SOVERSION 9)

add_executable(konsole1 ${KONSOLE_SRCS})
target_link_libraries(konsole1 libkonsole Qt::Qt1 ${X11_LIBRARIES} ${KDE1_KDECORE} ${KDE1_KIMGIO} ${KDE1_KDEUI})
install(TARGETS konsole1 RUNTIME DESTINATION ${KDE1_BINDIR})

QT1_WRAP_HEADER(KCMKONSOLE_MOC_FILES kcmkonsole.moc)
add_executable(kcmkonsole1 kcmkonsole.C schema.C ${KCMKONSOLE_MOC_FILES})
target_link_libraries(kcmkonsole1 Qt::Qt1 ${KDE1_KDECORE} ${KDE1_KIMGIO} ${KDE1_KDEUI})
install(TARGETS kcmkonsole1 RUNTIME DESTINATION ${KDE1_BINDIR})
