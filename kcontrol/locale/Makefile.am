
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
	$(MOC) locale.h -o locale.moc

klangcombo.cpp: klangcombo.moc
klangcombo.moc: klangcombo.h
	$(MOC) klangcombo.h -o klangcombo.moc

install-data-local: 
	$(mkinstalldirs) $(kde_confdir)
	$(INSTALL_DATA) kcmlocale.kdelnk $(kde_confdir)/kcmlocalerc
	$(mkinstalldirs) $(kde_icondir)
	$(INSTALL_DATA) locale.xpm $(kde_icondir)
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) mini-locale.xpm $(kde_minidir)/locale.xpm

uninstall-local:
	rm -f $(kde_confdir)/kcmlocalerc
	rm -f $(kde_icondir)/locale.xpm
	rm -f $(kde_minidir)/locale.xpm

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(kcmlocale_SOURCES) && mv messages.po ../../po/kcmlocale.pot

DISTCLEANFILES = $(BUILT_SOURCES)

