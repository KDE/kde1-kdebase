INCLUDES = $(all_includes) 
LDADD = $(all_libraries) -lkdecore -lqt 

bin_PROGRAMS = kcmsample

kcmsample_SOURCES = main.cpp kcontrol.cpp
BUILT_SOURCED = kcontrol.moc

kcontrol.cpp: kcontrol.moc
kcontrol.moc: kcontrol.h
	$(MOC) kcontrol.h -o kcontrol.moc

install-data-local: 
#	$(mkinstalldirs) $(kde_icondir)
#	$(INSTALL_DATA) kcontrol.xpm $(kde_icondir)/kcontrol.xpm
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) mini-sample.xpm $(kde_minidir)/sample.xpm

uninstall-local:
#	rm -f $(APPSDIR)/kcontrol.kdelnk
#	rm -f $(kde_icondir)/kcontrol.xpm
	rm -f $(kde_minidir)/sample.xpm

DISTCLEANFILES = $(BUILT_SOURCES)

