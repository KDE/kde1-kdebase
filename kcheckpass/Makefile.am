## Makefile.am for kcheckpass
## written by Christian Esken
##

INCLUDES= $(all_includes)
LDADD	= $(all_libraries) $(LIBCRYPT) $(PAMLIBS)

# for shadow passwords override the value
INSTALL_PROGRAM = $(INSTALL) $(SETUIDFLAGS)

bin_PROGRAMS = kcheckpass

kcheckpass_SOURCES = kcheckpass.cpp \
	checkpass_etcpasswd.cpp checkpass_pam.cpp checkpass_shadow.cpp

noinst_HEADERS = kcheckpass.h
