# $Id$
# kdebase/kcontrol/input

SUBDIRS = pics

INCLUDES = $(all_includes) 
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD =       $(LIB_KDEUI) 

bin_PROGRAMS = kcminput

kcminput_SOURCES = main.cpp mouse.cpp keyboard.cpp
BUILT_SOURCES = mouse.moc keyboard.moc
noinst_HEADERS = mouse.h keyboard.h geom.h

EXTRA_DIST = mouse.xpm mini-mouse.xpm keyboard.xpm mini-keyboard.xpm

mouse.cpp: mouse.moc geom.h
mouse.moc: mouse.h
	$(MOC) mouse.h -o mouse.moc

keyboard.cpp: keyboard.moc geom.h
keyboard.moc: keyboard.h
	$(MOC) keyboard.h -o keyboard.moc

install-data-local: 
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) mouse.xpm $(kde_icondir)/mouse.xpm
	$(INSTALL_DATA) keyboard.xpm $(kde_icondir)/keyboard.xpm
	$(INSTALL_DATA) mini-mouse.xpm $(kde_minidir)/mouse.xpm
	$(INSTALL_DATA) mini-keyboard.xpm $(kde_minidir)/keyboard.xpm

uninstall-local:
	rm -f $(kde_icondir)/mouse.xpm
	rm -f $(kde_icondir)/keyboard.xpm
	rm -f $(kde_minidir)/mouse.xpm
	rm -f $(kde_minidir)/keyboard.xpm

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(kcminput_SOURCES) && mv messages.po ../../po/kcminput.pot

DISTCLEANFILES = $(BUILT_SOURCES)
