## Makefile.am for kcheckpass
## written by Christian Esken
##

INCLUDES= $(all_includes)
LDADD	= $(all_libraries) $(LIBCRYPT) $(PAMLIBS)

# for shadow passwords override the value
INSTALL_PROGRAM = $(INSTALL) $(SETUIDFLAGS)

bin_PROGRAMS = kcheckpass

kcheckpass_SOURCES = kcheckpass.c \
	checkpass_etcpasswd.c checkpass_pam.c checkpass_shadow.c

noinst_HEADERS = kcheckpass.h

EXTRA_DIST = README-1.0

