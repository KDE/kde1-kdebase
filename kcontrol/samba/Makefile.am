INCLUDES = $(all_includes) 
LDADD = $(all_libraries) -lkdeui -lkdecore -lqt -lXext -lX11 $(LIBSOCKET) $(KDE_RPATH)

bin_PROGRAMS = kcmsamba

kcmsamba_SOURCES = main.cpp ksmbstatus.cpp
BUILT_SOURCED = ksmbstatus.moc

ksmbstatus.cpp: ksmbstatus.moc 
ksmbstatus.moc: ksmbstatus.h
	$(MOC) ksmbstatus.h -o ksmbstatus.moc

install-data-local: 
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) mini-smbstatus.xpm $(kde_minidir)/smbstatus.xpm

uninstall-local:
	rm -f $(kde_minidir)/smbstatus.xpm

messages:
	$(XGETTEXT) -C -ktranslate -x ../../../kdelibs/po/kde.pot $(kcmsamba_SOURCES) && mv messages.po ../../po/kcmsamba.pot

DISTCLEANFILES = $(BUILT_SOURCES)

