## Makefile.am for kfontmanager

###################################################################

LDADD   =	@all_libraries@ -lkdecore -lqt -lm -lXext -lX11
INCLUDES=	@all_includes@
MOC	=	@MOC@

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

