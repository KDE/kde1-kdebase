INCLUDES = $(all_includes) 
LDADD = $(all_libraries) -lkdeui -lkdecore -lqt -lXext -lX11 $(LIBSOCKET) $(KDE_RPATH)

noinst_PROGRAMS = kcmsample

kcmsample_SOURCES = main.cpp

EXTRA_DIST = mini-sample.xpm

install-data-local: 
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) mini-sample.xpm $(kde_minidir)/sample.xpm

uninstall-local:
	rm -f $(kde_minidir)/sample.xpm

messages:
	$(XGETTEXT) -C -ktranslate -x ../../../kdelibs/po/kde.pot $(kcmsample_SOURCES) && mv messages.po ../../po/kcmsample.pot

DISTCLEANFILES = $(BUILT_SOURCES)

