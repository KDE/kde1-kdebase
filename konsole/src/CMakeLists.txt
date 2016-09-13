include_directories(
    ${QT_INCLUDE_DIR}
    ${KDE1_INCLUDE_DIR}
    )

set(LIBKONSOLE_SRCS TEWidget.C TEmuVt102.C TEScreen.C TEmulation.C TEShell.C)
add_library(libkonsole STATIC ${LIBKONSOLE_SRCS})

#install(FILES kimgio.h DESTINATION ${KDE1_INCLUDES})

set(KONSOLE_SRCS main.C schema.C session.C)

add_executable(konsole ${KONSOLE_SRCS})
add_dependencies(konsole moc_ALL)
set_target_properties(konsole PROPERTIES CMAKE_EXE_LINKER_FLAGS "-version-info 9:1.9")
target_link_libraries(konsole ${QT_LIBRARIES} ${X11_LIBRARIES} ${LIB_KIMGIO} ${LIB_KDEUI})
install(TARGETS konsole RUNTIME DESTINATION bin)


