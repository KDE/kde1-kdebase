
INCLUDES = $(all_includes) 
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD = -lkdeui -lkdecore -lqt  -lXext -lX11 $(LIBSOCKET) 

bin_PROGRAMS = kcminfo

kcminfo_SOURCES = main.cpp memory.cpp
BUILT_SOURCES = memory.moc info.moc
noinst_HEADERS = info.h memory.h
##  memory_linux.h memory_fbsd.h memory_hpux.h memory_sgi.h info_linux.h info_fbsd.h info_hpux.h info_sgi.h

EXTRA_DIST = mini-kcmmemory.xpm mini-kcmprocessor.xpm mini-kcmdevices.xpm mini-kcmpartitions.xpm mini-kcmscsi.xpm mini-kcmsound.xpm mini-kcmx.xpm

memory.cpp: memory.moc
memory.moc: memory.h
	$(MOC) memory.h -o memory.moc

info.cpp: info.moc
info.moc: info.h
	$(MOC) info.h -o info.moc

main.cpp: info.moc

install-data-local: 
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) mini-kcmmemory.xpm     $(kde_minidir)/kcmmemory.xpm
	$(INSTALL_DATA) mini-kcmprocessor.xpm  $(kde_minidir)/kcmprocessor.xpm
	$(INSTALL_DATA) mini-kcmdevices.xpm    $(kde_minidir)/kcmdevices.xpm
	$(INSTALL_DATA) mini-kcmpartitions.xpm $(kde_minidir)/kcmpartitions.xpm
	$(INSTALL_DATA) mini-kcmscsi.xpm       $(kde_minidir)/kcmscsi.xpm
	$(INSTALL_DATA) mini-kcmsound.xpm      $(kde_minidir)/kcmsound.xpm
	$(INSTALL_DATA) mini-kcmx.xpm          $(kde_minidir)/kcmx.xpm

uninstall-local:
	rm -f $(kde_minidir)/kcmmemory.xpm
	rm -f $(kde_minidir)/kcmprocessor.xpm
	rm -f $(kde_minidir)/kcmdevices.xpm
	rm -f $(kde_minidir)/kcmpartitions.xpm
	rm -f $(kde_minidir)/kcmscsi.xpm
	rm -f $(kde_minidir)/kcmsound.xpm
	rm -f $(kde_minidir)/kcmx.xpm
	
messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x ../../../kdelibs/po/kde.pot $(kcminfo_SOURCES) $(noinst_HEADERS) && mv messages.po ../../po/kcminfo.pot

DISTCLEANFILES = $(BUILT_SOURCES)

