## Makefile.am for kdebase/kdehelp/rman

VOLLIST = "1:2:3:4:5:6:7:8:9:o:l:n:p"
MANTITLEPRINTF = "%s(%s)"
MANREFPRINTF = "man:%s(%s)"
version = 3.0.7

INCLUDES =  $(all_includes)
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD = $(LIBCOMPAT) 
CFLAGS = $(CFLAGS) -DVOLLIST='$(VOLLIST)' \
	-DMANTITLEPRINTF='$(MANTITLEPRINTF)' \
	-DMANREFPRINTF='$(MANREFPRINTF)' -DNDEBUG \
	-DPOLYGLOTMANVERSION=\"$(version)\"
#bindir = $(kde_cgidir)

bin_PROGRAMS = rman
rman_SOURCES = rman.c getopt.c

EXTRA_DIST = Copyright MANIFEST README-rman rman.1 rman.html

