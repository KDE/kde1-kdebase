
INCLUDES = $(all_includes) 
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD = -lkdeui -lkdecore  -lXext  $(LIB_QT)

noinst_PROGRAMS = kcmsample

kcmsample_SOURCES = main.cpp

EXTRA_DIST = mini-sample.xpm

install-data-local: 
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) mini-sample.xpm $(kde_minidir)/sample.xpm

uninstall-local:
	rm -f $(kde_minidir)/sample.xpm

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(kcmsample_SOURCES) && mv messages.po ../../po/kcmsample.pot

DISTCLEANFILES = $(BUILT_SOURCES)

