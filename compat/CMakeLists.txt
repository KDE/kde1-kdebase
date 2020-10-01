find_package(PkgConfig QUIET)
if (PkgConfig_FOUND)
    pkg_check_modules(libsystemd IMPORTED_TARGET libsystemd)
endif()
if(TARGET PkgConfig::libsystemd)
    message(STATUS "Enabling locking on suspend")
    add_executable(suspendlocker suspendlocker.cpp ../kscreensaver/kscreensave.cpp)
    target_link_libraries(suspendlocker PRIVATE
        Qt::Qt1
        X11::X11
        ${KDE1_KDECORE}
        ${KDE1_KDEUI}
        PkgConfig::libsystemd
        )
    target_include_directories(suspendlocker PRIVATE ../kscreensaver/ ${KDE1_INCLUDE_DIR})
    set_property(TARGET suspendlocker PROPERTY CXX_STANDARD 11)
else()
    message(STATUS "systemd not available, won't be able to lock on suspend")
endif()

# A modern session file to be compatible with modern login managers
install(FILES kde1.desktop DESTINATION share/xsessions)
