
INCLUDES=       $(all_includes) -I..
LDADD   =       $(all_libraries) -lkfm -lkdecore -lqt -lXext -lX11 $(LIBSOCKET) $(KDE_RPATH)

bin_PROGRAMS = kfmclient

kfmclient_SOURCES = kfmclient.cpp
noinst_HEADERS = 

kfmclient_DEPENDENCIES = debug.o

BUILT_SOURCES =	kfmclient_ipc.moc
DISTCLEANFILES = $(BUILT_SOURCES)

debug.o: ../debug.cpp ../config-kfm.h
	$(CXXCOMPILE) -c ../debug.cpp

####### Meta objects

