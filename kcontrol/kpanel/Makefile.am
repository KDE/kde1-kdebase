
INCLUDES = $(all_includes) 
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD =       $(LIB_KDEUI)

bin_PROGRAMS = kcmkpanel

kcmkpanel_SOURCES = main.cpp panel.cpp desktops.cpp options.cpp disknav.cpp
METASOURCES = panel.moc desktops.moc options.moc disknav.moc
noinst_HEADERS = panel.h desktops.h options.h disknav.h

messages:
	$(XGETTEXT) -C -ki18n -x $(includedir)/kde.pot $(kcmkpanel_SOURCES) && mv messages.po ../../po/kcmkpanel.pot

kde_icon_DATA = kcmkpanel.xpm

install-data-local:
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) $(srcdir)/mini-kcmkpanel.xpm $(kde_minidir)/kcmkpanel.xpm

EXTRA_DIST = mini-kcmkpanel.xpm kcmkpanel.xpm

uninstall-local:
	-rm -f $(kde_minidir)/kcmkpanel.xpm

