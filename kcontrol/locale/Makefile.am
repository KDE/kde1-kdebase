
INCLUDES = $(all_includes) -I..
LDFLAGS  = $(all_libraries) $(KDE_RPATH)
LDADD    = $(LIB_KFM) $(LIB_KDEUI) 

SUBDIRS = pics

bin_PROGRAMS = kcmlocale

kcmlocale_SOURCES = main.cpp locale.cpp klangcombo.cpp
METASOURCES = locale.moc klangcombo.moc
noinst_HEADERS = locale.h klangcombo.h
EXTRA_DIST = kcmlocale.kdelnk locale.xpm mini-locale.xpm large-locale.xpm

kde_icon_DATA = locale.xpm

install-data-local: 
	$(mkinstalldirs) $(kde_confdir)
	$(INSTALL_DATA) $(srcdir)/kcmlocale.kdelnk $(kde_confdir)/kcmlocalerc
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) $(srcdir)/mini-locale.xpm $(kde_minidir)/locale.xpm
	$(mkinstalldirs) $(kde_minidir)/../large/
	$(INSTALL_DATA) $(srcdir)/large-locale.xpm $(kde_minidir)/../large/locale.xpm

uninstall-local:
	rm -f $(kde_confdir)/kcmlocalerc
	rm -f $(kde_minidir)/locale.xpm
	rm -f $(kde_minidir)/../large/locale.xpm

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(kcmlocale_SOURCES) && mv messages.po ../../po/kcmlocale.pot


