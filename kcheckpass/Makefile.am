## Makefile.am for kvt 
## written by Stephan Kulow using the old Makefile.in, that I wrote too :-)
##

INCLUDES= $(all_includes)
LDADD	= $(all_libraries)

# for shadow passwords override the value
INSTALL_PROGRAM = @INSTALL@ $(SETUIDFLAGS)

bin_PROGRAMS = kcheckpass

kcheckpass_SOURCES = kcheckpass.cpp \
	checkpass_etcpasswd.cpp checkpass_pam.cpp checkpass_shadow.cpp

noinst_HEADERS = kcheckpass.h
