
INCLUDES = $(all_includes) -I..
LDFLAGS  = $(all_libraries) $(KDE_RPATH)
LDADD    = $(LIB_KFM) $(LIB_KDEUI) 

SUBDIRS = pics

bin_PROGRAMS = kcmlocale

kcmlocale_SOURCES = main.cpp locale.cpp klangcombo.cpp
BUILT_SOURCES = locale.moc
noinst_HEADERS = locale.h klangcombo.h
EXTRA_DIST = kcmlocalerc locale.xpm mini-locale.xpm

locale.cpp: locale.moc
locale.moc: locale.h
	$(MOC) $(srcdir)/locale.h -o locale.moc

klangcombo.cpp: klangcombo.moc
klangcombo.moc: klangcombo.h
	$(MOC) $(srcdir)/klangcombo.h -o klangcombo.moc

kde_icon_DATA = locale.xpm

install-data-local: 
	$(mkinstalldirs) $(kde_confdir)
	$(INSTALL_DATA) $(srcdir)/kcmlocale.kdelnk $(kde_confdir)/kcmlocalerc
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) $(srcdir)/mini-locale.xpm $(kde_minidir)/locale.xpm

uninstall-local:
	rm -f $(kde_confdir)/kcmlocalerc
	rm -f $(kde_minidir)/locale.xpm

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(kcmlocale_SOURCES) && mv messages.po ../../po/kcmlocale.pot

DISTCLEANFILES = $(BUILT_SOURCES)

