INCLUDES = $(all_includes) 
LDADD = $(all_libraries) -lkdeui -lkdecore -lqt -lXext -lX11 $(LIBSOCKET) $(KDE_RPATH)

bin_PROGRAMS = kcmkwm

kcmkwm_SOURCES = main.cpp titlebar.cpp windows.cpp desktop.cpp mouse.cpp
BUILT_SOURCES = titlebar.moc windows.moc desktop.moc mouse.moc
noinst_HEADERS = windows.h desktop.h titlebar.h geom.h mouse.h

EXTRA_DIST = mini-windows.xpm mini-titlebar.xpm mini-buttons.xpm \
		kcmkwm.xpm mini-winprops.xpm

windows.cpp: windows.moc
windows.moc: windows.h
	$(MOC) windows.h -o windows.moc


desktop.cpp: desktop.moc
desktop.moc: desktop.h
	$(MOC) desktop.h -o desktop.moc

titlebar.cpp: titlebar.moc
titlebar.moc: titlebar.h
	$(MOC) titlebar.h -o titlebar.moc

mouse.cpp: mouse.moc
mouse.moc: mouse.h
	$(MOC) mouse.h -o mouse.moc

install-data-local: 
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) mini-windows.xpm $(kde_minidir)/windows.xpm
	$(INSTALL_DATA) mini-titlebar.xpm $(kde_minidir)/titlebar.xpm
	$(INSTALL_DATA) mini-buttons.xpm $(kde_minidir)/buttons.xpm
	$(INSTALL_DATA) mini-winprops.xpm $(kde_minidir)/winprops.xpm
	$(INSTALL_DATA) kcmkwm.xpm $(kde_icondir)/kcmkwm.xpm

uninstall-local:
	rm -f $(kde_minidir)/windows.xpm
	rm -f $(kde_minidir)/titlebar.xpm
	rm -f $(kde_minidir)/buttons.xpm
	rm -f $(kde_icondir)/kcmkwm.xpm

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(kcmkwm_SOURCES) && mv messages.po ../../po/kcmkwm.pot

DISTCLEANFILES = $(BUILT_SOURCES)

