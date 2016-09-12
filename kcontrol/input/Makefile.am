# $Id: Makefile.am,v 1.16 1999/01/18 12:46:17 kulow Exp $
# kdebase/kcontrol/input

SUBDIRS = pics

INCLUDES = $(all_includes) 
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD =       $(LIB_KDEUI) 

bin_PROGRAMS = kcminput

kcminput_SOURCES = main.cpp mouse.cpp keyboard.cpp kslidercontrol.cpp
METASOURCES = mouse.moc keyboard.moc kslidercontrol.moc
noinst_HEADERS = mouse.h keyboard.h geom.h kslidercontrol.h

EXTRA_DIST = mouse.xpm mini-mouse.xpm keyboard.xpm mini-keyboard.xpm

kde_icon_DATA = mouse.xpm keyboard.xpm

install-data-local: 
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) $(srcdir)/mini-mouse.xpm $(kde_minidir)/mouse.xpm
	$(INSTALL_DATA) $(srcdir)/mini-keyboard.xpm $(kde_minidir)/keyboard.xpm

uninstall-local:
	rm -f $(kde_minidir)/mouse.xpm
	rm -f $(kde_minidir)/keyboard.xpm

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(kcminput_SOURCES) && mv messages.po ../../po/kcminput.pot

