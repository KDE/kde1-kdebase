INCLUDES = $(all_includes) 
LDADD = $(all_libraries) -lkdecore -lqt -lkdeui

bin_PROGRAMS = kcmbell

kcmbell_SOURCES = main.cpp kcontrol.cpp bell.cpp
BUILT_SOURCED = kcontrol.moc bell.moc

kcontrol.cpp: kcontrol.moc
kcontrol.moc: kcontrol.h
	$(MOC) kcontrol.h -o kcontrol.moc

bell.cpp: bell.moc
bell.moc: bell.h
	$(MOC) bell.h -o bell.moc

install-data-local: 
#	$(mkinstalldirs) $(kde_icondir)
#	$(INSTALL_DATA) kcontrol.xpm $(kde_icondir)/kcontrol.xpm
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) mini-bell.xpm $(kde_minidir)/bell.xpm

uninstall-local:
#	rm -f $(APPSDIR)/kcontrol.kdelnk
#	rm -f $(kde_icondir)/kcontrol.xpm
	rm -f $(kde_minidir)/bell.xpm

DISTCLEANFILES = $(BUILT_SOURCES)

