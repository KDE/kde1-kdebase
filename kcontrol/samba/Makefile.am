INCLUDES = $(all_includes) 
LDADD = $(all_libraries) -lkdecore -lkdeui -lqt 

bin_PROGRAMS = kcmsamba

kcmsamba_SOURCES = main.cpp kcontrol.cpp ksmbstatus.cpp ktablistbox.cpp
BUILT_SOURCED = kcontrol.moc ksmbstatus.moc ktablistbox.moc

kcontrol.cpp: kcontrol.moc
kcontrol.moc: kcontrol.h
	$(MOC) kcontrol.h -o kcontrol.moc

ksmbstatus.cpp: ksmbstatus.moc 
ksmbstatus.moc: ksmbstatus.h
	$(MOC) ksmbstatus.h -o ksmbstatus.moc

ktablistbox.cpp: ktablistbox.moc
ktablistbox.moc: ktablistbox.h
	$(MOC) ktablistbox.h -o ktablistbox.moc

install-data-local: 
#	$(mkinstalldirs) $(kde_icondir)
#	$(INSTALL_DATA) kcontrol.xpm $(kde_icondir)/kcontrol.xpm
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) mini-smbstatus.xpm $(kde_minidir)/smbstatus.xpm

uninstall-local:
#	rm -f $(kde_icondir)/kcontrol.xpm
	rm -f $(kde_minidir)/smbstatus.xpm

DISTCLEANFILES = $(BUILT_SOURCES)

