# Makefile.am for kdebase/kaudio
# $Id$

INCLUDES = $(all_includes)
LDADD =  $(all_libraries) -lkfile -lkfm -lmediatool -lkdeui -lkdecore -lqt -lXext -lX11 $(LIBSOCKET) $(LIBUCB) $(KDE_RPATH) $(LIBCOMPAT)

maudio_SOURCES = io_oss.cpp maudio.cpp sample.cpp
kaudioserver_SOURCES = kaudioserver.cpp
demo_SOURCES = democlient.cpp
kplayaudio_SOURCES = kplayaudio.cpp
MOCS = democlient.moc kplayaudio.moc
DISTCLEANFILES = $(MOCS)

bin_PROGRAMS = maudio kaudioserver kplayaudio
noinst_PROGRAMS = demo
noinst_HEADERS = version.h maudio.h sample.h io_oss.h democlient.h kplayaudio.h

datadir = $(kde_htmldir)/en/kmedia/
data_DATA = maudio.html

EXTRA_DIST = $(data_DATA) VERSION

version.h: VERSION
	sed "s/0/#define APP_VERSION 0/" <VERSION >version.h

democlient.cpp: democlient.moc

democlient.moc: democlient.h
	$(MOC) -o democlient.moc democlient.h

kplayaudio.cpp: kplayaudio.moc

kplayaudio.moc: kplayaudio.h
	$(MOC) -o kplayaudio.moc kplayaudio.h


messages:
