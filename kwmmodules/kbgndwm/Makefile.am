####### Fiddle here

INCLUDES = $(all_includes)
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD   =   $(LIB_KIMGIO)     $(LIB_KDEUI) 

#######	Files

bin_PROGRAMS	= kbgndwm
kbgndwm_SOURCES = main.cpp kbgndwm.cpp bg.cpp 
METASOURCES = kbgndwm.moc bg.moc
noinst_HEADERS = kbgndwm.h bg.h version.h config-kbgndwm.h

EXTRA_DIST = README

messages:
	$(XGETTEXT) -C -ki18n -x $(includedir)/kde.pot $(kbgndwm_SOURCES) && mv messages.po ../../po/kbgndwm.pot
