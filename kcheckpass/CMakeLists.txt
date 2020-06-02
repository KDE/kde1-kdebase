include_directories(
    ${PNG_INCLUDE_DIR}
    )

set(KCHECKPASS_SRCS kcheckpass.c checkpass_etcpasswd.c
    checkpass_pam.c checkpass_shadow.c checkpass_osfc2passwd.c)

add_executable(kcheckpass ${KCHECKPASS_SRCS})
target_link_libraries(kcheckpass crypt Qt::Qt1)

install(TARGETS kcheckpass RUNTIME  DESTINATION ${KDE1_BINDIR})
