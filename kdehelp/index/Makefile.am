## Makefile.am for kdebase/kdehelp/index

INCLUDES =  $(all_includes)
LDADD = $(all_libraries) -lkdecore -lqt -lXext -lX11 $(LIBSOCKET) $(LIBCOMPAT) $(KDE_RPATH)
bindir = $(kde_cgidir)

bin_PROGRAMS = helpindex
helpindex_SOURCES = kdeindex.cpp

messages:
	$(XGETTEXT) -C -ktranslate -kaliasLocale -ki18n -x $(includedir)/kde.pot $(helpindex_SOURCES) && mv messages.po ../../po/helpindex.pot
