APPSDIR = $(kde_appsdir)/Settings/Desktop
INCLUDES= $(all_includes)
THEMESDIR= $(kde_datadir)/kthememgr/Themes

####### Files

#SUBDIRS = 
bin_PROGRAMS = 	kthememgr
kthememgr_SOURCES = theme.cpp installer.cpp main.cpp kwmcom.cpp \
	options.cpp widgetcanvas.cpp colors.cpp about.cpp groupdetails.cpp
noinst_HEADERS = theme.h installer.h kwmcom.h widgetcanvas.h \
	options.h colors.h about.h version.h groupdetails.h
kthememgr_LDADD = $(LIB_KIMGIO) -lkfm     -lkfile $(LIB_KDEUI)
kthememgr_LDFLAGS = $(all_libraries) $(LDFLAGS) $(KDE_RPATH)

BUILTSOURCES = theme.moc installer.moc colors.moc widgetcanvas.moc \
	options.moc about.moc groupdetails.moc
DISTCLEANFILES = theme.moc installer.moc colors.moc widgetcanvas.moc \
	options.moc about.moc groupdetails.moc

install-data-local: 
	$(mkinstalldirs) $(APPSDIR)
	$(INSTALL_DATA) kthememgr.kdelnk $(APPSDIR)
	$(mkinstalldirs) $(kde_icondir)
	$(INSTALL_DATA) kthememgr.xpm $(kde_icondir)
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) mini-kthememgr.xpm $(kde_minidir)/kthememgr.xpm
	$(mkinstalldirs) $(kde_datadir)/kthememgr
	$(INSTALL_DATA) theme.mappings $(kde_datadir)/kthememgr
	$(mkinstalldirs) $(THEMESDIR)
	$(INSTALL_DATA) Themes/*.tar.gz $(THEMESDIR)
	$(INSTALL_DATA) Themes/Template.themerc $(THEMESDIR)

uninstall-local:
	rm -f $(APPSDIR)/kthememgr.kdelnk
	rm -f $(kde_icondir)/kthememgr.xpm
	rm -f $(kde_minidir)/kthememgr.xpm
	rm -f $(kde_datadir)/kthememgr/theme.mappings
	rm -f $(THEMESDIR)/Template.themerc

messages:
	@XGETTEXT@ -C -ki18n *.cpp -x../../kdelibs/po/kde.pot && \
	mv messages.po ../po/kthememgr.pot

edit:
	xemacs $(kthememgr_SOURCES) $(noinst_HEADERS) &

theme.cpp: theme.moc
theme.moc: theme.h
	$(MOC) theme.h -o theme.moc

installer.cpp: installer.moc
installer.moc: installer.h
	$(MOC) installer.h -o installer.moc

widgetcanvas.cpp: widgetcanvas.moc
widgetcanvas.moc: widgetcanvas.h
	$(MOC) widgetcanvas.h -o widgetcanvas.moc

colors.cpp: colors.moc
colors.moc: colors.h
	$(MOC) colors.h -o colors.moc

options.cpp: options.moc
options.moc: options.h
	$(MOC) options.h -o options.moc

about.cpp: about.moc
about.moc: about.h
	$(MOC) about.h -o about.moc

groupdetails.cpp: groupdetails.moc
groupdetails.moc: groupdetails.h
	$(MOC) groupdetails.h -o groupdetails.moc
