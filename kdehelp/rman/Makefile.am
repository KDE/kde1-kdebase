## Makefile.am for kdebase/kdehelp/rman

VOLLIST = "1:2:3:4:5:6:7:8:9:o:l:n:p"
MANTITLEPRINTF = "%s(%s)"
MANREFPRINTF = "man:%s(%s)"
version = 2.5a6

INCLUDES =  $(all_includes)
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD = $(LIBCOMPAT) 
CFLAGS = -DVOLLIST='$(VOLLIST)' -DMANTITLEPRINTF='$(MANTITLEPRINTF)' -DMANREFPRINTF='$(MANREFPRINTF)' -DNDEBUG -DROSETTAMANVERSION=\"$(version)\"
#bindir = $(kde_cgidir)

bin_PROGRAMS = rman
rman_SOURCES = rman.c getopt.c

