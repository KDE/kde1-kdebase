
INCLUDES = $(all_includes) 
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD = $(LIB_KDEUI)

bin_PROGRAMS = kcminfo

SUBDIRS = pics

kcminfo_SOURCES = main.cpp memory.cpp
METASOURCES = memory.moc info.moc
noinst_HEADERS = info.h memory.h
##  memory_linux.h memory_fbsd.h memory_hpux.h memory_sgi.h info_linux.h info_fbsd.h info_hpux.h info_sgi.h

# main.cpp includes info.cpp. make can't know this
main.cpp: info.cpp

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x ../../../kdelibs/po/kde.pot *.cpp && mv messages.po ../../po/kcminfo.pot

