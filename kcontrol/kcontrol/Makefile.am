# $Id$
# kdebase/kcontrol/kcontrol

INCLUDES = $(all_includes) 
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD =       $(LIBCOMPAT) $(LIB_KDEUI) 

SUBDIRS = pics

bin_PROGRAMS = kcontrol

kcontrol_SOURCES = configlist.cpp main.cpp toplevel.cpp kdelnk.cpp mainwidget.cpp \
		   kswallow.cpp
BUILT_SOURCES =	toplevel.moc kdelnk.moc mainwidget.moc configlist.moc \
		kswallow.moc
noinst_HEADERS = kswallow.h toplevel.h configlist.h mainwidget.h kdelnk.h

EXTRA_DIST = mini-kcontrol.xpm

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(kcontrol_SOURCES) && mv messages.po ../../po/kcontrol.pot

kswallow.cpp: kswallow.moc
kswallow.moc: kswallow.h
	$(MOC) $(srcdir)/kswallow.h -o kswallow.moc

toplevel.cpp: toplevel.moc
toplevel.moc: toplevel.h
	$(MOC) $(srcdir)/toplevel.h -o toplevel.moc

configlist.cpp: configlist.moc
configlist.moc: configlist.h
	$(MOC) $(srcdir)/configlist.h -o configlist.moc

mainwidget.cpp: mainwidget.moc
mainwidget.moc: mainwidget.h
	$(MOC) $(srcdir)/mainwidget.h -o mainwidget.moc

kdelnk.cpp: kdelnk.moc
kdelnk.moc: kdelnk.h
	$(MOC) $(srcdir)/kdelnk.h -o kdelnk.moc

install-data-local: 
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) $(srcdir)/mini-kcontrol.xpm $(kde_minidir)/kcontrol.xpm

uninstall-local:
	rm -f $(kde_minidir)/kcontrol.xpm


DISTCLEANFILES = $(BUILT_SOURCES)
