INCLUDES = $(all_includes) 
LDADD = $(all_libraries) -lkdecore -lkdeui -lqt 

bin_PROGRAMS = kcminput

kcminput_SOURCES = main.cpp kcontrol.cpp mouse.cpp keyboard.cpp
BUILT_SOURCED = kcontrol.moc mouse.moc keyboard.moc

kcontrol.cpp: kcontrol.moc
kcontrol.moc: kcontrol.h
	$(MOC) kcontrol.h -o kcontrol.moc

mouse.cpp: mouse.moc geom.h
mouse.moc: mouse.h
	$(MOC) mouse.h -o mouse.moc

keyboard.cpp: keyboard.moc geom.h
keyboard.moc: keyboard.h
	$(MOC) keyboard.h -o keyboard.moc

install-data-local: 
#	$(mkinstalldirs) $(kde_icondir)
#	$(INSTALL_DATA) kcontrol.xpm $(kde_icondir)/kcontrol.xpm
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) mini-mouse.xpm $(kde_minidir)/mouse.xpm
	$(INSTALL_DATA) mini-keyboard.xpm $(kde_minidir)/keyboard.xpm

uninstall-local:
#	rm -f $(kde_icondir)/kcontrol.xpm
	rm -f $(kde_minidir)/mouse.xpm
	rm -f $(kde_minidir)/keyboard.xpm

DISTCLEANFILES = $(BUILT_SOURCES)

