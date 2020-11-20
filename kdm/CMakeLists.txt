add_subdirectory(pics)
add_subdirectory(config)
add_subdirectory(xdmconfig)

include_directories(
    ${KDE1_INCLUDE_DIR}
    )

set(kdmlib_SRC
    access.c
    auth.c
    choose.c
    daemon.c
    dm.c
    dpylist.c
    error.c
    file.c
    genauth.c
    mitauth.c
    netaddr.c
    policy.c
    protodpy.c
    reset.c
    resource.c
    rpcauth.c
    server.c
    session.c
    socket.c
    streams.c
    util.c
    verify.c
    xdmauth.c
    xdmcp.c
    )

qt1_wrap_moc(MOC_SRCS
        kdmshutdown.h
        kdmview.h
        kfdialog.h
        kgreeter.h
)

add_executable(kdm
    kdmconfig.cpp
    kdmshutdown.cpp
    kdmview.cpp
    kfdialog.cpp
    kgreeter.cpp
    ${MOC_SRCS}
    ${kdmlib_SRC}
)

add_executable(kdmdesktop
    kdmdesktop.cpp
    )
#target_compile_definitions(kdm PRIVATE XDMDIR="${XDMDIR}")
#target_compile_definitions(kdmdesktop PRIVATE XDMDIR="${XDMDIR}")

target_link_libraries(kdm
    Qt::Qt1
    X11::X11
    X11::Xdmcp
    X11::Xau
    PkgConfig::libcrypt
    ${PAM_LIBRARIES}
    ${KDE1_KDECORE}
    ${KDE1_KFILE}
    ${KDE1_KIMGIO}
    ${KDE1_KDEUI}
    )

target_link_libraries(kdmdesktop
    X11::X11
    Qt::Qt1
    ${KDE1_KDECORE}
    ${KDE1_KIMGIO}
    )

if (PAM_FOUND)
    target_compile_definitions(kdm PRIVATE USE_PAM)
endif()

install(TARGETS kdm RUNTIME DESTINATION ${KDE1_BINDIR})
