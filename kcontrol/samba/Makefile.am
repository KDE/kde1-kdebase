
INCLUDES = $(all_includes) 
LDFLAGS  = $(all_libraries) $(KDE_RPATH)
LDADD    = $(LIB_KDEUI) 

bin_PROGRAMS = kcmsamba

kcmsamba_SOURCES = main.cpp ksmbstatus.cpp
METASOURCES = ksmbstatus.moc
noinst_HEADERS = ksmbstatus.h

EXTRA_DIST = smbstatus.xpm mini-smbstatus.xpm

kde_icon_DATA = smbstatus.xpm

install-data-local:
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) $(srcdir)/mini-smbstatus.xpm $(kde_minidir)/smbstatus.xpm

uninstall-local:
	rm -f $(kde_minidir)/smbstatus.xpm

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(kcmsamba_SOURCES) && mv messages.po ../../po/kcmsamba.pot

