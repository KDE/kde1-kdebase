## Makefile.am for kfontmanager

###################################################################

LDADD   =       $(LIB_KDECORE) -lm
LDFLAGS = 	$(all_libraries) $(KDE_RPATH)
INCLUDES=	$(all_includes)

####### Files
bin_PROGRAMS = kfontmanager
noinst_HEADERS = kfontmanager.h kfontlist.h kfontexplorer.h version.h
kfontmanager_SOURCES = main.cpp kfontmanager.cpp kfontlist.cpp kfontexplorer.cpp
METASOURCES =	kfontmanager.moc kfontlist.moc kfontexplorer.moc

EXTRA_DIST = kfontmanager.xpm mini-kfontmanager.xpm

####### Meta objects

install-data-local:
	$(mkinstalldirs) $(kde_icondir)
	$(INSTALL_DATA) $(srcdir)/kfontmanager.xpm $(kde_icondir)/kfontmanager.xpm
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) $(srcdir)/mini-kfontmanager.xpm $(kde_minidir)/kfontmanager.xpm

uninstall-local:
	rm -f $(kde_icondir)/kfontmanager.xpm
	rm -f $(kde_minidir)/kfontmanager.xpm

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(kfontmanager_SOURCES) && mv messages.po ../po/kfontmanager.pot

