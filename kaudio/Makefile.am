
INCLUDES = $(all_includes)
# kdecore for usleep fake
LDADD =  $(all_libraries) -lmediatool -lkdeui -lkdecore -lqt -lXext -lX11 $(LIBSOCKET) $(LIBUCB) $(KDE_RPATH)

maudio_SOURCES = io_oss.cpp maudio.cpp sample.cpp
kaudioserver_SOURCES = kaudioserver.cpp
demo_SOURCES = democlient.cpp
MOCS = democlient.moc
DISTCLEANFILES = $(MOCS)

bin_PROGRAMS = maudio kaudioserver
noinst_PROGRAMS = demo
noinst_HEADERS = version.h

datadir = $(kde_htmldir)/en/kmedia/
data_DATA = maudio.html

version.h: VERSION
	sed "s/0/#define APP_VERSION 0/" <VERSION >version.h

democlient.cpp: democlient.moc

democlient.moc: democlient.h
	$(MOC) -o democlient.moc democlient.h

messages:
