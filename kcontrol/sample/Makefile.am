
INCLUDES = $(all_includes) 
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD =       $(LIB_KDEUI)

noinst_PROGRAMS = kcmsample

kcmsample_SOURCES = main.cpp

EXTRA_DIST = mini-sample.xpm

install-data-local: 
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) $(srcdir)/mini-sample.xpm $(kde_minidir)/sample.xpm

uninstall-local:
	rm -f $(kde_minidir)/sample.xpm

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(kcmsample_SOURCES) && mv messages.po ../../po/kcmsample.pot

DISTCLEANFILES = $(BUILT_SOURCES)

