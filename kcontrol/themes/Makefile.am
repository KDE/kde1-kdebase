APPSDIR = $(kde_appsdir)/Settings/Desktop
INCLUDES= $(all_includes)
THEMESDIR= $(kde_datadir)/kthememgr/Themes

####### Files

SUBDIRS = doc
bin_PROGRAMS = 	kthememgr
kthememgr_SOURCES = theme.cpp installer.cpp main.cpp kwmcom.cpp \
	options.cpp widgetcanvas.cpp about.cpp groupdetails.cpp \
	themecreator.cpp newthemedlg.cpp
noinst_HEADERS = theme.h installer.h kwmcom.h widgetcanvas.h \
	options.h colors.h about.h version.h groupdetails.h \
	themecreator.h newthemedlg.h

kthememgr_LDADD = $(LIB_KIMGIO) $(LIB_KFILE) 
kthememgr_LDFLAGS = $(all_libraries) $(LDFLAGS) $(KDE_RPATH)

METASOURCES = theme.moc installer.moc colors.moc widgetcanvas.moc \
	options.moc about.moc groupdetails.moc themecreator.moc

kde_icon_DATA = kthememgr.xpm

install-data-local: install-themes
	$(mkinstalldirs) $(APPSDIR)
	$(INSTALL_DATA) $(srcdir)/kthememgr.kdelnk $(APPSDIR)
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) $(srcdir)/mini-kthememgr.xpm $(kde_minidir)/kthememgr.xpm
	$(mkinstalldirs) $(kde_datadir)/kthememgr
	$(INSTALL_DATA) $(srcdir)/theme.mappings $(kde_datadir)/kthememgr

uninstall-local:
	rm -f $(APPSDIR)/kthememgr.kdelnk
	rm -f $(kde_minidir)/kthememgr.xpm
	rm -f $(kde_datadir)/kthememgr/theme.mappings
	rm -f $(THEMESDIR)/Template.themerc

install-themes:
	$(mkinstalldirs) $(THEMESDIR)
	$(INSTALL_DATA) $(srcdir)/Themes/Template.themerc $(THEMESDIR)
	cd $(srcdir)/Themes; \
	for f in *; do \
	  if [ -d $$f -a $$f != "CVS" ]; then \
	    $(TAR) --exclude CVS -c -f - $$f | gzip -c > $(THEMESDIR)/$$f.tar.gz; \
	  fi; \
	done

messages:
	$(XGETTEXT) -C -ki18n -x $(includedir)/kde.pot $(kthememgr_SOURCES) && mv messages.po ../../po/kthememgr.pot

edit:
	xemacs $(kthememgr_SOURCES) $(noinst_HEADERS) &

