INCLUDES = $(all_includes) 
LDADD = $(all_libraries) -lkdecore -lkdeui -lqt 

bin_PROGRAMS = kcmsamba

kcmsamba_SOURCES = main.cpp ksmbstatus.cpp ktablistbox.cpp
BUILT_SOURCED = ksmbstatus.moc ktablistbox.moc

ksmbstatus.cpp: ksmbstatus.moc 
ksmbstatus.moc: ksmbstatus.h
	$(MOC) ksmbstatus.h -o ksmbstatus.moc

ktablistbox.cpp: ktablistbox.moc
ktablistbox.moc: ktablistbox.h
	$(MOC) ktablistbox.h -o ktablistbox.moc

install-data-local: 
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) mini-smbstatus.xpm $(kde_minidir)/smbstatus.xpm

uninstall-local:
	rm -f $(kde_minidir)/smbstatus.xpm

DISTCLEANFILES = $(BUILT_SOURCES)

