
INCLUDES = $(all_includes) 
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD = $(LIB_KDEUI)

bin_PROGRAMS = kcminfo

SUBDIRS = pics

kcminfo_SOURCES = main.cpp memory.cpp
METASOURCES = memory.moc info.moc
noinst_HEADERS = info.h memory.h info.cpp info_fbsd.cpp \
	info_generic.cpp info_hpux.cpp info_linux.cpp \
	info_sgi.cpp info_svr4.cpp main.cpp memory.cpp memory_fbsd.cpp \
	memory_hpux.cpp memory_linux.cpp memory_sgi.cpp 

# main.cpp includes info.cpp. make can't know this
main.cpp: info.cpp

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x ../../../kdelibs/po/kde.pot *.cpp && mv messages.po ../../po/kcminfo.pot

