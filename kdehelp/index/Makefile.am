## Makefile.am for kdebase/kdehelp/index

INCLUDES =  $(all_includes)
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD = $(LIB_KDECORE) $(LIBCOMPAT) 
bindir = $(kde_cgidir)

bin_PROGRAMS = helpindex
helpindex_SOURCES = kdeindex.cpp

messages:
	$(XGETTEXT) -C -ktranslate -kaliasLocale -ki18n -x $(includedir)/kde.pot $(helpindex_SOURCES) && mv messages.po ../../po/helpindex.pot
