
INCLUDES = $(all_includes)

kpager_LDFLAGS	= $(all_libraries) $(KDE_RPATH)
kpager_LDADD    = $(LIB_KFILE) $(LIB_KIMGIO) 

kpager_SOURCES = windowdrag.cpp ktrianglebutton.cpp desktop.cpp kpagerclient.cpp kpager.cpp main.cpp

kpager_METASOURCES = USE_AUTOMOC 

DISTCLEANFILES = $(kpager_METASOURCES) ./.deps/* ./.deps/.P

bin_PROGRAMS = kpager

applnk_DATA = kpager.kdelnk
applnkdir = $(kde_appsdir)/System

messages:
	$(XGETTEXT) -C -ki18n -x $(includedir)/kde.pot $(kpager_SOURCES) && mv messages.po ../../po/kpager.pot
