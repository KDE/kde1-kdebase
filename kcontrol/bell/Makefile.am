
INCLUDES = $(all_includes) 
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD =    $(LIB_KDEUI) 

bin_PROGRAMS = kcmbell

kcmbell_SOURCES = main.cpp bell.cpp kslidercontrol.cpp
METASOURCES = bell.moc kslidercontrol.moc
noinst_HEADERS = bell.h geom.h kslidercontrol.h

EXTRA_DIST = bell.xpm mini-bell.xpm large-bell.xpm

kde_icon_DATA = bell.xpm

install-data-local: 
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) $(srcdir)/mini-bell.xpm $(kde_minidir)/bell.xpm
	$(mkinstalldirs) $(kde_minidir)/../large/
	$(INSTALL_DATA) $(srcdir)/large-bell.xpm $(kde_minidir)/../large/bell.xpm

uninstall-local:
	rm -f $(kde_minidir)/bell.xpm
	rm -f $(kde_minidir)/../large/bell.xpm

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(kcmbell_SOURCES) && mv messages.po ../../po/kcmbell.pot

