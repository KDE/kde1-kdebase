
INCLUDES = $(all_includes) 
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD =       $(LIB_KDEUI)

bin_PROGRAMS = kcmkpanel

kcmkpanel_SOURCES = main.cpp panel.cpp desktops.cpp options.cpp disknav.cpp
BUILT_SOURCES = panel.moc desktops.moc options.moc disknav.moc
noinst_HEADERS = panel.h desktops.h options.h disknav.h

messages:
	$(XGETTEXT) -C -ki18n -x $(includedir)/kde.pot $(kcmkpanel_SOURCES) && mv messages.po ../../po/kcmkpanel.pot

icon_DATA = kcmkpanel.xpm
icondir = $(kde_icondir)

install-data-local:
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) mini-kcmkpanel.xpm $(kde_minidir)/kcmkpanel.xpm

uninstall-local:
	-rm -f $(kde_minidir)/kcmkpanel.xpm

DISTCLEANFILES = $(BUILT_SOURCES)

panel.cpp:panel.moc
panel.moc: panel.h
	$(MOC) panel.h -o panel.moc

desktops.cpp:desktops.moc
desktops.moc: desktops.h
	$(MOC) desktops.h -o desktops.moc

options.cpp:options.moc
options.moc: options.h
	$(MOC) options.h -o options.moc

disknav.cpp:disknav.moc
disknav.moc: disknav.h
	$(MOC) disknav.h -o disknav.moc
