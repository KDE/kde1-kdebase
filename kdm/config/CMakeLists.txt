configure_file(kdmrc.in ${CMAKE_CURRENT_BINARY_DIR}/kdmrc)
install(FILES kdmrc DESTINATION ${KDE1_CONFDIR})

