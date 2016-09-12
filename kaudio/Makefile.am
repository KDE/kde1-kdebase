# Makefile.am for kdebase/kaudio
# $Id: Makefile.am,v 1.28 1999/01/22 17:36:56 kulow Exp $

INCLUDES = $(all_includes)
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD =   -lmediatool       $(LIB_KFILE) $(LIBUCB) $(LIBCOMPAT)

maudio_SOURCES = io_oss.cpp maudio.cpp sample.cpp
kaudioserver_SOURCES = kaudioserver.cpp
demo_SOURCES = democlient.cpp
kplayaudio_SOURCES = kplayaudio.cpp
METASOURCES = democlient.moc kplayaudio.moc

bin_PROGRAMS = maudio kaudioserver kplayaudio
noinst_PROGRAMS = demo
noinst_HEADERS = version.h maudio.h sample.h io_oss.h democlient.h kplayaudio.h

datadir = $(kde_htmldir)/en/kmedia/
data_DATA = maudio.html

EXTRA_DIST = $(data_DATA) VERSION README.kaudioserver kaudioserver.lsm maudio.lsm 

version.h: VERSION
	sed "s/0/#define APP_VERSION 0/" <VERSION >version.h

messages:
