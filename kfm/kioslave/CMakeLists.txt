include_directories(
    ${QT_INCLUDE_DIR}
    ${KDE1_INCLUDE_DIR}
    )

set(KIOSLAVE_SRCS
kioslave_ipc.cpp kioslave_ipc2.cpp main.cpp xview.cpp
	ipc.cpp protocol.cpp http.cpp file.cpp ftp.cpp manage.cpp icon.cpp
	slave.cpp tar.cpp subprotocol.cpp cgi.cpp gzip.cpp ftp_proxy.cpp ../debug.cpp)

set(KIOSLAVE_MOC kioslave_ipc.moc main.moc http.moc file.moc tar.moc ftp.moc protocol.moc subprotocol.moc cgi.moc gzip.moc icon.moc ftp_proxy.moc)

QT1_WRAP_MOC(MOC_FILES ${KIOSLAVE_MOC})

add_executable(kioslave ${KIOSLAVE_SRCS} ${MOC_FILES})
target_link_libraries(kioslave
    ${QT_LIBRARIES}
    ${X11_LIBRARIES}
    ${KDE1_KDECORE}
    ${KDE1_KDEUI}
    ${KDE1_KIMGIO}
    )

install(TARGETS kioslave RUNTIME DESTINATION ${KDE1_BINDIR})
