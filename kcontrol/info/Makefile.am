INCLUDES = $(all_includes) 
LDADD = $(all_libraries) -lkdeui -lkdecore -lqt  -lXext -lX11 $(LIBSOCKET) $(KDE_RPATH)

bin_PROGRAMS = kcminfo

kcminfo_SOURCES = main.cpp memory.cpp processor.cpp
BUILT_SOURCES = memory.moc processor.moc
noinst_HEADERS = processor.h memory.h memory_linux.cpp memory_fbsd.cpp memory_hpux.cpp memory_sgi.cpp processor_linux.cpp processor_fbsd.cpp processor_hpux.cpp processor_sgi.cpp

EXTRA_DIST = mini-memory.xpm mini-processor.xpm

memory.cpp: memory.moc
memory.moc: memory.h
	$(MOC) memory.h -o memory.moc

processor.cpp: processor.moc
processor.moc: processor.h
	$(MOC) processor.h -o processor.moc

install-data-local: 
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) mini-memory.xpm $(kde_minidir)/memory.xpm
	$(INSTALL_DATA) mini-processor.xpm $(kde_minidir)/processor.xpm

uninstall-local:
	rm -f $(kde_minidir)/memory.xpm
	rm -f $(kde_minidir)/processor.xpm

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x ../../../kdelibs/po/kde.pot $(kcminfo_SOURCES) $(noinst_HEADERS) && mv messages.po ../../po/kcminfo.pot

DISTCLEANFILES = $(BUILT_SOURCES)

