INCLUDES = $(all_includes) 
LDADD = $(all_libraries) -lkdeui -lkdecore -lqt -lXext -lX11 $(LIBSOCKET) $(KDE_RPATH)

bin_PROGRAMS = kcmkpanel

kcmkpanel_SOURCES = main.cpp panel.cpp desktops.cpp
BUILT_SOURCED = panel.moc desktops.moc
noinst_HEADERS = panel.h desktops.h

messages:
	$(XGETTEXT) -C -ktranslate -x ../../../kdelibs/po/kde.pot $(kcmkpanel_SOURCES) && mv messages.po ../../po/kcmkpanel.pot

DISTCLEANFILES = $(BUILT_SOURCES)

panel.cpp:panel.moc
panel.moc: panel.h
	$(MOC) panel.h -o panel.moc

desktops.cpp:desktops.moc
desktops.moc: desktops.h
	$(MOC) desktops.h -o desktops.moc

