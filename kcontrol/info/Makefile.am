INCLUDES = $(all_includes) 
LDADD = $(all_libraries) -lkdeui -lkdecore -lqt  -lXext -lX11 $(LIBSOCKET) $(KDE_RPATH)

bin_PROGRAMS = kcminfo

kcminfo_SOURCES = main.cpp memory.cpp processor.cpp
BUILT_SOURCED = memory.moc processor.moc

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
	$(XGETTEXT) -C -ktranslate -x ../../../kdelibs/po/kde.pot $(kcminfo_SOURCES) && mv messages.po ../../po/kcminfo.pot

DISTCLEANFILES = $(BUILT_SOURCES)

