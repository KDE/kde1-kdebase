
INCLUDES =  $(all_includes)
LDADD = $(all_libraries) -lkfm -lkhtmlw -ljscript -lkdeui -lkdecore -lqt -lXext -lX11 $(LIBSOCKET) $(LIBCOMPAT) $(LIBJPEG) $(KDE_RPATH)
SUBDIRS = search index

########## Files ##########

bin_PROGRAMS = kdehelp
kdehelp_SOURCES =  main.cpp helpwin.cpp info.cpp error.cpp misc.cpp options.cpp\
		man.cpp bookmark.cpp cgi.cpp mainwidget.cpp history.cpp
BUILT_SOURCES = helpwin.moc options.moc bookmark.moc cgi.moc mainwidget.moc
DISTCLEANFILES = $(BUILT_SOURCES)

noinst_HEADERS = helpwin.h info.h error.h misc.h man.h options.h\
		bookmark.h dbnew.cpp cgi.h mainwidget.h history.h

helpwin.cpp: helpwin.moc
options.cpp: options.moc
bookmark.cpp: bookmark.moc
cgi.cpp: cgi.moc
mainwidget.cpp: mainwidget.moc

########## Meta objects ##########

helpwin.moc: helpwin.h
	$(MOC) helpwin.h -o helpwin.moc

options.moc: options.h
	$(MOC) options.h -o options.moc

bookmark.moc: bookmark.h
	$(MOC) bookmark.h -o bookmark.moc

cgi.moc: cgi.h
	$(MOC) cgi.h -o cgi.moc

mainwidget.moc: mainwidget.h
	$(MOC) mainwidget.h -o mainwidget.moc

install-data-local:
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) kdehelp.xpm $(kde_icondir)/kdehelp.xpm
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) mini-kdehelp.xpm $(kde_minidir)/kdehelp.xpm

uninstall-local:
	rm -f $(kde_icondir)/kdehelp.xpm
	rm -f $(kde_minidir)/kdehelp.xpm

messages:
	$(XGETTEXT) -C -ktranslate -kaliasLocale -x ../../kdelibs/po/kde.pot $(kdehelp_SOURCES) && mv messages.po ../po/kdehelp.pot
