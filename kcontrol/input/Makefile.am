INCLUDES = $(all_includes) 
LDADD = $(all_libraries) -lkdecore -lkdeui -lqt 

bin_PROGRAMS = kcminput

kcminput_SOURCES = main.cpp mouse.cpp keyboard.cpp
BUILT_SOURCED = mouse.moc keyboard.moc

mouse.cpp: mouse.moc geom.h
mouse.moc: mouse.h
	$(MOC) mouse.h -o mouse.moc

keyboard.cpp: keyboard.moc geom.h
keyboard.moc: keyboard.h
	$(MOC) keyboard.h -o keyboard.moc

install-data-local: 
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) mini-mouse.xpm $(kde_minidir)/mouse.xpm
	$(INSTALL_DATA) mini-keyboard.xpm $(kde_minidir)/keyboard.xpm

uninstall-local:
	rm -f $(kde_minidir)/mouse.xpm
	rm -f $(kde_minidir)/keyboard.xpm

DISTCLEANFILES = $(BUILT_SOURCES)

