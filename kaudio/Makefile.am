
INCLUDES = $(all_includes)
# kdecore for usleep fake
LDADD =  $(all_libraries) -lmediatool -lkdecore $(KDE_RPATH)

maudio_SOURCES = io_oss.cpp maudio.cpp sample.cpp
kaudioserver_SOURCES = kaudioserver.cpp
demo_SOURCES = democlient.cpp

bin_PROGRAMS = maudio kaudioserver
noinst_PROGRAMS = demo
noinst_HEADERS = version.h

datadir = $(kde_htmldir)/en/kmedia/
data_DATA = maudio.html

version.h: VERSION
	sed "s/0/#define VERSION 0/" <VERSION >version.h

