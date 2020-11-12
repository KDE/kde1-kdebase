add_subdirectory(pics)
add_subdirectory(apps)

find_package(PkgConfig QUIET)
if (PkgConfig_FOUND)
    pkg_check_modules(libsystemd IMPORTED_TARGET libsystemd)
endif()
if(TARGET PkgConfig::libsystemd)
    find_package(Threads)
    find_package(X11)

    message(STATUS "Enabling locking on suspend")
    add_executable(kde1-screenlockinvoker kde1-screenlockinvoker.cpp ../kscreensaver/kscreensave.cpp)

    # needs std::make_unique from c++14
    set_property(TARGET kde1-screenlockinvoker PROPERTY CXX_STANDARD 14)

    target_link_libraries(kde1-screenlockinvoker PRIVATE
        Qt::Qt1
        X11::X11
        X11::Xss
        ${KDE1_KDECORE}
        ${KDE1_KDEUI}
        PkgConfig::libsystemd
        Threads::Threads
        )
    target_include_directories(kde1-screenlockinvoker PRIVATE ../kscreensaver/ ${KDE1_INCLUDE_DIR})

    install(TARGETS kde1-screenlockinvoker RUNTIME DESTINATION ${KDE1_BINDIR})
else()
    message(STATUS "systemd not available, won't be able to lock on suspend")
endif()

# Start script without all the sleeps
install(FILES startkde1-fast PERMISSIONS WORLD_EXECUTE WORLD_READ DESTINATION ${KDE1_BINDIR})

# A modern session file to be compatible with modern login managers
install(FILES kde1.desktop DESTINATION share/xsessions)
