
INCLUDES =  $(all_includes)
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD = $(LIB_KFILE) $(LIB_KHTMLW) $(LIBCOMPAT)
SUBDIRS = search index rman

########## Files ##########

bin_PROGRAMS	=	kdehelp

kdehelp_SOURCES	=	main.cpp helpwin.cpp info.cpp error.cpp misc.cpp \
		options.cpp man.cpp bookmark.cpp cgi.cpp mainwidget.cpp \
		history.cpp booktoken.cpp finddlg.cpp

kdehelp_METASOURCES = 	USE_AUTOMOC

noinst_HEADERS =	helpwin.h info.h error.h misc.h man.h options.h\
		bookmark.h dbnew.cpp cgi.h mainwidget.h history.h\
		booktoken.h fmthtml.h fmtbase.h dbnew.h khelp.h finddlg.h

EXTRA_DIST	=	kdehelp.xpm mini-kdehelp.xpm CHANGES form.html test.html

kde_icon_DATA = kdehelp.xpm

install-data-local:
	$(mkinstalldirs) $(kde_minidir)
	$(mkinstalldirs) $(kde_icondir)/large
	$(INSTALL_DATA) $(srcdir)/mini-kdehelp.xpm $(kde_minidir)/kdehelp.xpm
	$(INSTALL_DATA) $(srcdir)/large-kdehelp.xpm $(kde_icondir)/large/kdehelp.xpm

uninstall-local:
	rm -f $(kde_icondir)/large/kdehelp.xpm
	rm -f $(kde_minidir)/kdehelp.xpm

messages:
	$(XGETTEXT) -C -ktranslate -kaliasLocale -ki18n -x $(includedir)/kde.pot $(kdehelp_SOURCES) && mv messages.po ../po/kdehelp.pot

