INCLUDES = $(all_includes) 
LDADD = $(all_libraries) -lkdecore -lqt  -lkdeui

bin_PROGRAMS = kcminfo

kcminfo_SOURCES = main.cpp kcontrol.cpp memory.cpp processor.cpp
BUILT_SOURCED = kcontrol.moc memory.moc processor.moc

kcontrol.cpp: kcontrol.moc
kcontrol.moc: kcontrol.h
	$(MOC) kcontrol.h -o kcontrol.moc

memory.cpp: memory.moc
memory.moc: memory.h
	$(MOC) memory.h -o memory.moc

processor.cpp: processor.moc
processor.moc: processor.h
	$(MOC) processor.h -o processor.moc

install-data-local: 
#	$(mkinstalldirs) $(kde_icondir)
#	$(INSTALL_DATA) kcontrol.xpm $(kde_icondir)/kcontrol.xpm
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) mini-memory.xpm $(kde_minidir)/memory.xpm
	$(INSTALL_DATA) mini-processor.xpm $(kde_minidir)/processor.xpm

uninstall-local:
#	rm -f $(APPSDIR)/kcontrol.kdelnk
#	rm -f $(kde_icondir)/kcontrol.xpm
	rm -f $(kde_minidir)/memory.xpm
	rm -f $(kde_minidir)/processor.xpm

DISTCLEANFILES = $(BUILT_SOURCES)

