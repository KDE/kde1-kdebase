MOC = @MOC@
INCLUDES =  @all_includes@  
LDADD = @all_libraries@ -lkfm @LIBKHTMLW@ -lkdeui -lkdecore -lqt -lXext -lX11 @LIBSOCKET@ @LIBCOMPAT@ -lgif @LIBJPEG@

SUBDIRS = search index

prefix = @prefix@
BINDIR = $(prefix)/bin
CGIDIR = $(prefix)/cgi-bin
LIBDIR = $(prefix)/lib
APPSDIR = $(LIBDIR)/apps
PICSDIR = $(LIBDIR)/pics
DOCSDIR = $(prefix)/doc/HTML

########## Files ##########

bin_PROGRAMS = kdehelp

kdehelp_SOURCES =  main.cpp helpwin.cpp info.cpp error.cpp misc.cpp options.cpp\
		man.cpp bookmark.cpp cgi.cpp mainwidget.cpp 
BUILTSOURCES = helpwin.moc options.moc bookmark.moc cgi.moc mainwidget.moc
		
noinst_HEADERS = helpwin.h info.h error.h misc.h man.h options.h\
		bookmark.h dbnew.cpp cgi.h mainwidget.h 

helpwin.cpp: helpwin.moc
options.cpp: options.moc
bookmark.cpp: bookmark.moc
cgi.cpp: cgi.moc
mainwidget.cpp: mainwidget.moc

install-data-local: all
	(cd doc && $(MAKE) install)
	$(mkinstalldirs) $(PICSDIR)
	$(INSTALL_DATA) pics/khtmlw_dnd.xpm $(PICSDIR)
	$(INSTALL_DATA) pics/stop.xpm $(PICSDIR)
	$(INSTALL_DATA) pics/stop_gray.xpm $(PICSDIR)

uninstall-local:
	(cd doc && $(MAKE) uninstall)
	rm -f $(PICSDIR)/khtmlw_dnd.xpm
	rm -f $(PICSDIR)/stop.xpm
	rm -f $(PICSDIR)/stop_gray.xpm

########## Meta objects ##########

%.moc: %.h
	$(MOC) $< -o $@

