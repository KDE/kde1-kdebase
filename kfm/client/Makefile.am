
INCLUDES=       $(all_includes) -I..
LDFLAGS = 	$(all_libraries) $(KDE_RPATH)
LDADD   =       -lkfm -lkdecore -lqt -lXext -lX11 $(LIBSOCKET) 

bin_PROGRAMS = kfmclient

kfmclient_SOURCES = kfmclient.cpp
noinst_HEADERS = 

kfmclient_DEPENDENCIES = debug.o

BUILT_SOURCES =
DISTCLEANFILES = $(BUILT_SOURCES)

debug.o: ../debug.cpp ../config-kfm.h
	$(CXXCOMPILE) -c ../debug.cpp

####### Meta objects

