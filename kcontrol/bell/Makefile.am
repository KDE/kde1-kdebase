
INCLUDES = $(all_includes) 
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD = -lkdeui -lkdecore  -lXext  $(LIB_QT) 

bin_PROGRAMS = kcmbell

kcmbell_SOURCES = main.cpp bell.cpp
BUILT_SOURCES = bell.moc
noinst_HEADERS = bell.h geom.h

EXTRA_DIST = bell.xpm mini-bell.xpm

bell.cpp: bell.moc
bell.moc: bell.h
	$(MOC) bell.h -o bell.moc

install-data-local: 
	$(mkinstalldirs) $(kde_icondir)
	$(INSTALL_DATA) bell.xpm $(kde_icondir)
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) mini-bell.xpm $(kde_minidir)/bell.xpm

uninstall-local:
	rm -f $(kde_icondir)/bell.xpm
	rm -f $(kde_minidir)/bell.xpm

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(kcmbell_SOURCES) && mv messages.po ../../po/kcmbell.pot

DISTCLEANFILES = $(BUILT_SOURCES)

