INCLUDES = $(all_includes) 
LDADD = $(all_libraries) -lkdeui -lkdecore -lqt -lXext -lX11 $(LIBSOCKET) $(KDE_RPATH)

bin_PROGRAMS = kcmsample

kcmsample_SOURCES = main.cpp

install-data-local: 
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) mini-sample.xpm $(kde_minidir)/sample.xpm

uninstall-local:
	rm -f $(kde_minidir)/sample.xpm

DISTCLEANFILES = $(BUILT_SOURCES)

