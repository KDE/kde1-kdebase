## Makefile.am for kfontmanager

APPSDIR = $(prefix)/apps/System

###################################################################

LDADD   =	@all_libraries@ -lkdecore -lqt -lm -lXext -lX11
INCLUDES=	@all_includes@
MOC	=	@MOC@

####### Files
bin_PROGRAMS = kfontmanager
noinst_HEADERS = kfontmanager.h kfontlist.h kfontexplorer.h
kfontmanager_SOURCES = main.cpp kfontmanager.cpp kfontlist.cpp kfontexplorer.cpp
BUILTSOURCES =	kfontmanager.moc kfontlist.moc kfontexplorer.moc

kfontmanager.cpp: kfontmanager.moc
kfontlist.cpp: kfontlist.moc
kfontexplorer.cpp: kfontexplorer.moc

install-local-data:
	(cd docs && $(MAKE) install)
	$(mkinstalldirs) $(APPSDIR)
	$(INSTALL_DATA) kfontmanager.kdelnk $(APPSDIR)/kfontmanager.kdelnk

uninstall-local:
	(cd docs && $(MAKE) install)
	rm -f $(APPSDIR)/kfontmanager.kdelnk

####### Meta objects

kfontmanager.moc: kfontmanager.h
	$(MOC) kfontmanager.h -o kfontmanager.moc

kfontexplorer.moc: kfontexplorer.h
	$(MOC) kfontexplorer.h -o kfontexplorer.moc

kfontlist.moc: kfontlist.h
	$(MOC) kfontlist.h -o kfontlist.moc

