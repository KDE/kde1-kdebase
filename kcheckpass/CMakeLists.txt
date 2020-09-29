include_directories(
    ${PNG_INCLUDE_DIR}
    )

set(KCHECKPASS_SRCS kcheckpass.c)

if (PAM_FOUND)
    set(KCHECKPASS_SRCS ${KCHECKPASS_SRCS} checkpass_pam.c)
else()
    set(KCHECKPASS_SRCS ${KCHECKPASS_SRCS} checkpass_shadow.c)
endif()
# checkpass_etcpasswd.c checkpass_shadow.c checkpass_osfc2passwd.c

add_executable(kde1-kcheckpass ${KCHECKPASS_SRCS})
target_link_libraries(kde1-kcheckpass crypt Qt::Qt1)

if (PAM_FOUND)
    target_link_libraries(kde1-kcheckpass ${PAM_LIBRARIES})

    install(TARGETS kde1-kcheckpass RUNTIME DESTINATION ${KDE1_BINDIR})
else()
    message(WARNING "Did not find PAM, kcheckpass needs to be SUID")
    target_compile_definitions(kde1-kcheckpass PRIVATE HAVE_SHADOW)

    install(TARGETS kde1-kcheckpass RUNTIME
        DESTINATION ${KDE1_BINDIR}
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE SETUID
        )
endif()
