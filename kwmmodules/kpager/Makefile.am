
INCLUDES = $(all_includes)

kpager_LDFLAGS	= $(all_libraries) $(KDE_RPATH)
kpager_LDADD    = $(LIB_KFILE) $(LIB_KIMGIO) 

kpager_SOURCES = windowdrag.cpp ktrianglebutton.cpp desktop.cpp kpagerclient.cpp kpager.cpp main.cpp

noinst_HEADERS = desktop.h kpager.h kpagerclient.h ktrianglebutton.h \
	version.h windowdrag.h

kpager_METASOURCES = USE_AUTOMOC 

DISTCLEANFILES = ./.deps/* ./.deps/.P

EXTRA_DIST = CHANGES kpager.kdelnk

bin_PROGRAMS = kpager

applnk_DATA = kpager.kdelnk
applnkdir = $(kde_appsdir)/System

messages:
	$(XGETTEXT) -C -ki18n -x $(includedir)/kde.pot $(kpager_SOURCES) && mv messages.po ../../po/kpager.pot
