INCLUDES = $(all_includes) 
LDADD = $(all_libraries) -lkdeui -lkdecore -lqt -lXext -lX11 $(LIBSOCKET) $(KDE_RPATH)

bin_PROGRAMS = kcmkpanel

kcmkpanel_SOURCES = main.cpp panel.cpp desktops.cpp options.cpp
BUILT_SOURCES = panel.moc desktops.moc options.moc
noinst_HEADERS = panel.h desktops.h options.h

messages:
	$(XGETTEXT) -C -ki18n -x $(includedir)/kde.pot $(kcmkpanel_SOURCES) && mv messages.po ../../po/kcmkpanel.pot

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

