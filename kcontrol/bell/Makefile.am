INCLUDES = $(all_includes) 
LDADD = $(all_libraries) -lkdeui -lkdecore -lqt -lXext -lX11 $(LIBSOCKET) $(KDE_RPATH)

bin_PROGRAMS = kcmbell

kcmbell_SOURCES = main.cpp bell.cpp
BUILT_SOURCES = bell.moc

bell.cpp: bell.moc
bell.moc: bell.h
	$(MOC) bell.h -o bell.moc

install-data-local: 
	$(mkinstalldirs) $(kde_icondir)
	$(INSTALL_DATA) bell.xpm $(kde_icondir)
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) mini-bell.xpm $(kde_minidir)/bell.xpm

uninstall-local:
	rm -f $(kde_icondir)/bell.xpm
	rm -f $(kde_minidir)/bell.xpm

messages:
	$(XGETTEXT) -C -ktranslate -x ../../../kdelibs/po/kde.pot $(kcmbell_SOURCES) && mv messages.po ../../po/kcmbell.pot

DISTCLEANFILES = $(BUILT_SOURCES)

