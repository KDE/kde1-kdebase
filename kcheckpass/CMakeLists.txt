include_directories(
    ${PNG_INCLUDE_DIR}
    )

set(KCHECKPASS_SRCS kcheckpass.c checkpass_etcpasswd.c
    checkpass_pam.c checkpass_shadow.c checkpass_osfc2passwd.c)

add_executable(kde1-kcheckpass ${KCHECKPASS_SRCS})
target_link_libraries(kde1-kcheckpass crypt Qt::Qt1)

install(TARGETS kde1-kcheckpass RUNTIME
    DESTINATION ${KDE1_BINDIR}
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE SETUID
    )
