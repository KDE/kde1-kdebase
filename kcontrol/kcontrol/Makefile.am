# $Id: Makefile.am,v 1.13.4.2 1999/08/08 22:43:34 dmuell Exp $
# kdebase/kcontrol/kcontrol

INCLUDES = $(all_includes) 
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD =       $(LIBCOMPAT) $(LIB_KDEUI) 

SUBDIRS = pics

bin_PROGRAMS = kcontrol

kcontrol_SOURCES = configlist.cpp main.cpp toplevel.cpp kdelnk.cpp mainwidget.cpp \
		   kswallow.cpp
METASOURCES =	toplevel.moc kdelnk.moc mainwidget.moc configlist.moc \
		kswallow.moc
noinst_HEADERS = kswallow.h toplevel.h configlist.h mainwidget.h kdelnk.h

EXTRA_DIST = mini-kcontrol.xpm

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(kcontrol_SOURCES) && mv messages.po ../../po/kcontrol.pot

install-data-local: 
	$(mkinstalldirs) $(kde_minidir)
	$(mkinstalldirs) $(kde_icondir)
	$(INSTALL_DATA) $(srcdir)/mini-kcontrol.xpm $(kde_minidir)/kcontrol.xpm
	$(INSTALL_DATA) $(srcdir)/large-kcontrol.xpm $(kde_icondir)/large/kcontrol.xpm

uninstall-local:
	rm -f $(kde_icondir)/large/kcontrol.xpm
	rm -f $(kde_minidir)/kcontrol.xpm

