
INCLUDES=       @all_includes@ -I..
LDADD   =       @all_libraries@ -lkdecore -lqt -lXext -lX11 @LIBSOCKET@
MOC     =       @MOC@

bin_PROGRAMS = kfmclient

kfmclient_SOURCES = kfmclient.cpp kfmclient_ipc.cpp kfmclient_ipc2.cpp ipc.cpp ../debug.cpp
noinst_HEADERS = kfmclient_ipc.h ipc.h

kfmclient_DEPENDENCIES = debug.o

BUILT_SOURCES =	kfmclient_ipc.moc
DISTCLEANFILES = $(BUILT_SOURCES)

kfmclient_ipc.cpp: kfmclient_ipc.moc

debug.o: ../debug.cpp ../config-kfm.h
	$(CXXCOMPILE) -c ../debug.cpp

####### Meta objects

kfmclient_ipc.moc: kfmclient_ipc.h
	$(MOC) kfmclient_ipc.h -o kfmclient_ipc.moc

