## Makefile.am for kfontmanager

###################################################################

LDADD   =	$(all_libraries) -lkdecore -lqt -lm -lXext -lX11 $(KDE_RPATH) $(LIBSOCKET)
INCLUDES=	$(all_includes)

####### Files
bin_PROGRAMS = kfontmanager
noinst_HEADERS = kfontmanager.h kfontlist.h kfontexplorer.h
kfontmanager_SOURCES = main.cpp kfontmanager.cpp kfontlist.cpp kfontexplorer.cpp
BUILT_SOURCES =	kfontmanager.moc kfontlist.moc kfontexplorer.moc
DISTCLEANFILES = $(BUILT_SOURCES)

kfontmanager.cpp: kfontmanager.moc
kfontlist.cpp: kfontlist.moc
kfontexplorer.cpp: kfontexplorer.moc

####### Meta objects

kfontmanager.moc: kfontmanager.h
	$(MOC) kfontmanager.h -o kfontmanager.moc

kfontexplorer.moc: kfontexplorer.h
	$(MOC) kfontexplorer.h -o kfontexplorer.moc

kfontlist.moc: kfontlist.h
	$(MOC) kfontlist.h -o kfontlist.moc
	
install-data-local:
	$(mkinstalldirs) $(kde_icondir)
	$(INSTALL_DATA) kfontmanager.xpm $(kde_icondir)/kfontmanager.xpm
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) mini-kfontmanager.xpm $(kde_minidir)/kfontmanager.xpm

uninstall-local:
	rm -f $(kde_icondir)/kfontmanager.xpm
	rm -f $(kde_minidir)/kfontmanager.xpm

