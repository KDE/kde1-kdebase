INCLUDES = $(all_includes) 
LDADD = $(all_libraries) -lkdecore -lqt -lkdeui

bin_PROGRAMS = kcmkwm

kcmkwm_SOURCES = main.cpp kcontrol.cpp titlebar.cpp windows.cpp
BUILT_SOURCED = kcontrol.moc titlebar.moc windows.moc

kcontrol.cpp: kcontrol.moc
kcontrol.moc: kcontrol.h
	$(MOC) kcontrol.h -o kcontrol.moc

windows.cpp: windows.moc
windows.moc: windows.h
	$(MOC) windows.h -o windows.moc

titlebar.cpp: titlebar.moc
titlebar.moc: titlebar.h
	$(MOC) titlebar.h -o titlebar.moc

install-data-local: 
#	$(mkinstalldirs) $(kde_icondir)
#	$(INSTALL_DATA) kcontrol.xpm $(kde_icondir)/kcontrol.xpm
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) mini-windows.xpm $(kde_minidir)/windows.xpm
	$(INSTALL_DATA) mini-titlebar.xpm $(kde_minidir)/titlebar.xpm
	$(INSTALL_DATA) mini-buttons.xpm $(kde_minidir)/buttons.xpm

uninstall-local:
#	rm -f $(APPSDIR)/kcontrol.kdelnk
#	rm -f $(kde_icondir)/kcontrol.xpm
	rm -f $(kde_minidir)/windows.xpm
	rm -f $(kde_minidir)/titlebar.xpm
	rm -f $(kde_minidir)/buttons.xpm

DISTCLEANFILES = $(BUILT_SOURCES)

