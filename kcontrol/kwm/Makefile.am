INCLUDES = $(all_includes) 
LDADD = $(all_libraries) -lkdeui -lkdecore -lqt -lXext -lX11 $(LIBSOCKET) $(KDE_RPATH)

bin_PROGRAMS = kcmkwm

kcmkwm_SOURCES = main.cpp titlebar.cpp windows.cpp
BUILT_SOURCED = titlebar.moc windows.moc

windows.cpp: windows.moc
windows.moc: windows.h
	$(MOC) windows.h -o windows.moc

titlebar.cpp: titlebar.moc
titlebar.moc: titlebar.h
	$(MOC) titlebar.h -o titlebar.moc

install-data-local: 
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) mini-windows.xpm $(kde_minidir)/windows.xpm
	$(INSTALL_DATA) mini-titlebar.xpm $(kde_minidir)/titlebar.xpm
	$(INSTALL_DATA) mini-buttons.xpm $(kde_minidir)/buttons.xpm
	$(INSTALL_DATA) kcmkwm.xpm $(kde_icondir)/kcmkwm.xpm

uninstall-local:
	rm -f $(kde_minidir)/windows.xpm
	rm -f $(kde_minidir)/titlebar.xpm
	rm -f $(kde_minidir)/buttons.xpm
	rm -f $(kde_icondir)/kcmkwm.xpm

messages:
	$(XGETTEXT) -C -ktranslate -x ../../../kdelibs/po/kde.pot $(kcmkwm_SOURCES) && mv messages.po ../../po/kcmkwm.pot

DISTCLEANFILES = $(BUILT_SOURCES)

