## Makefile.am for kdebase/kdehelp/search

INCLUDES = $(all_includes)
LDADD = $(all_libraries) -lkdecore -lqt -lXext -lX11 -lstdc++ $(LIBSOCKET) $(LIBCOMPAT) $(KDE_RPATH)
bindir = $(kde_cgidir)

bin_PROGRAMS = helpsearch

helpsearch_SOURCES = search.cpp mansrch.cpp htmlsrch.cpp
noinst_HEADERS = mansrch.h htmlsrch.h


