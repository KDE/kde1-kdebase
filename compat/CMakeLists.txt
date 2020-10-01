find_package(PkgConfig QUIET)
if (PkgConfig_FOUND)
    pkg_check_modules(libsystemd IMPORTED_TARGET libsystemd)
endif()
if(TARGET PkgConfig::libsystemd)
    message(STATUS "Enabling locking on suspend")
    add_executable(kde1-suspendlocker suspendlocker.cpp ../kscreensaver/kscreensave.cpp)
    set_property(TARGET kde1-suspendlocker PROPERTY CXX_STANDARD 11)

    target_link_libraries(kde1-suspendlocker PRIVATE
        Qt::Qt1
        X11::X11
        ${KDE1_KDECORE}
        ${KDE1_KDEUI}
        PkgConfig::libsystemd
        )
    target_include_directories(kde1-suspendlocker PRIVATE ../kscreensaver/ ${KDE1_INCLUDE_DIR})

    install(TARGETS kde1-suspendlocker RUNTIME DESTINATION ${KDE1_BINDIR})
else()
    message(STATUS "systemd not available, won't be able to lock on suspend")
endif()

# Start script without all the sleeps
install(FILES startkde1-fast PERMISSIONS WORLD_EXECUTE WORLD_READ DESTINATION ${KDE1_BINDIR})

# A modern session file to be compatible with modern login managers
install(FILES kde1.desktop DESTINATION share/xsessions)
