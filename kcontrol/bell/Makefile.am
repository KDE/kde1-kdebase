INCLUDES = $(all_includes) 
LDADD = $(all_libraries) -lkdeui -lkdecore -lqt -lXext -lX11 $(LIBSOCKET) $(KDE_RPATH)

bin_PROGRAMS = kcmbell

kcmbell_SOURCES = main.cpp bell.cpp
BUILT_SOURCED = bell.moc

bell.cpp: bell.moc
bell.moc: bell.h
	$(MOC) bell.h -o bell.moc

install-data-local: 
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) mini-bell.xpm $(kde_minidir)/bell.xpm

uninstall-local:
	rm -f $(kde_minidir)/bell.xpm

DISTCLEANFILES = $(BUILT_SOURCES)

