
INCLUDES = $(all_includes) 
LDADD = $(all_libraries) -lkdeui -lkdecore -lqt -lXext -lX11 $(LIBCOMPAT) $(LIBSOCKET) $(KDE_RPATH)

SUBDIRS = pics

bin_PROGRAMS = kcontrol

kcontrol_SOURCES = configlist.cpp main.cpp toplevel.cpp kdelnk.cpp mainwidget.cpp \
		   kswallow.cpp
BUILT_SOURCES =	toplevel.moc kdelnk.moc mainwidget.moc configlist.moc \
		kswallow.cpp

messages:
	$(XGETTEXT) -C -ktranslate -x ../../../kdelibs/po/kde.pot $(kcontrol_SOURCES) && mv messages.po ../../po/kcontrol.pot

kswallow.cpp: kswallow.moc
kswallow.moc: kswallow.h
	$(MOC) kswallow.h -o kswallow.moc

toplevel.cpp: toplevel.moc
toplevel.moc: toplevel.h
	$(MOC) toplevel.h -o toplevel.moc

configlist.cpp: configlist.moc
configlist.moc: configlist.h
	$(MOC) configlist.h -o configlist.moc

mainwidget.cpp: mainwidget.moc
mainwidget.moc: mainwidget.h
	$(MOC) mainwidget.h -o mainwidget.moc

kdelnk.cpp: kdelnk.moc
kdelnk.moc: kdelnk.h
	$(MOC) kdelnk.h -o kdelnk.moc

install-data-local: 
	$(mkinstalldirs) $(kde_icondir)
	$(INSTALL_DATA) kcontrol.xpm $(kde_icondir)/kcontrol.xpm
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) mini-kcontrol.xpm $(kde_minidir)/kcontrol.xpm

uninstall-local:
	rm -f $(APPSDIR)/kcontrol.kdelnk
	rm -f $(kde_icondir)/kcontrol.xpm
	rm -f $(kde_minidir)/kcontrol.xpm


DISTCLEANFILES = $(BUILT_SOURCES)

