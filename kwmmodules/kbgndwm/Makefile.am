####### Fiddle here

INCLUDES = @all_includes@ 
LDADD   = @all_libraries@ -lkdeui -lkdecore -lqt -lXext -lX11 @LIBSOCKET@ -lgif @LIBJPEG@
MOC	=	@MOC@

#######	Files

bin_PROGRAMS	= kbgndwm
kbgndwm_SOURCES = main.cpp kbgndwm.cpp bg.cpp gif.cpp jpeg.cpp
BUILTSOURCES =	kbgndwm.moc
DISTCLEANFILES = $(BUILTSOURCES)

kbgndwm.cpp: kbgndwm.moc
kbgndwm.moc: kbgndwm.h
	$(MOC) kbgndwm.h -o kbgndwm.moc


