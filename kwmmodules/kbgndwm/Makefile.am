####### Fiddle here

INCLUDES = $(all_includes)
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD   = -lkdeui -lkdecore $(LIB_KIMGIO) -lqt -lXext -lX11 $(LIBSOCKET) 

#######	Files

bin_PROGRAMS	= kbgndwm
kbgndwm_SOURCES = main.cpp kbgndwm.cpp bg.cpp 
DISTCLEANFILES = kbgndwm.moc bg.moc
noinst_HEADERS = kbgndwm.h bg.h version.h

kbgndwm.cpp: kbgndwm.moc
kbgndwm.moc: kbgndwm.h
	$(MOC) kbgndwm.h -o kbgndwm.moc

bg.cpp: bg.moc
bg.moc: bg.h
	$(MOC) bg.h -o bg.moc

install-data-local:

uninstall-local:

