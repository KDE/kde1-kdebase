####### Fiddle here

INCLUDES = $(all_includes)
LDADD   = $(all_libraries) -lkdeui -lkdecore -lqt -lXext -lX11 $(LIBSOCKET) -lgif $(LIBJPEG) $(KDE_RPATH)

#######	Files

bin_PROGRAMS	= kbgndwm
kbgndwm_SOURCES = main.cpp kbgndwm.cpp bg.cpp gif.cpp jpeg.cpp
BUILTSOURCES =	kbgndwm.moc
DISTCLEANFILES = $(BUILTSOURCES)

kbgndwm.cpp: kbgndwm.moc
kbgndwm.moc: kbgndwm.h
	$(MOC) kbgndwm.h -o kbgndwm.moc

install-data-local:
	$(mkinstalldirs) $(kde_appsdir)/System
	$(INSTALL_DATA) kbgndwm.kdelnk $(kde_appsdir)/System

uninstall-local:
	-rm -f $(kde_appsdir)/System/kbgndwm.kdelnk

