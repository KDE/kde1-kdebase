configure_file(kappfinder.cmake.in kappfinder)
install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/kappfinder
        kappfinder_restart
    DESTINATION ${KDE1_BINDIR}
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)
