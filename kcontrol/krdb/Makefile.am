INCLUDES = $(all_includes) 
LDADD = $(all_libraries) -lkdeui -lkdecore -lqt -lXext -lX11 $(LIBJPEG) $(LIBSOCKET) $(KDE_RPATH)

DISTCLEANFILES = $(BUILT_SOURCES)

SUBDIRS = kcs ad

bin_PROGRAMS = kcmdisplay

kcmdisplay_SOURCES = backgnd.cpp jpeg.cpp display.cpp \
		colorscm.cpp savescm.cpp scrnsave.cpp \
		general.cpp main.cpp widgetcanvas.cpp \
		fontchooser.cpp kresourceman.cpp
BUILT_SOURCES = backgnd.moc display.moc colorscm.moc savescm.moc \
		scrnsave.moc general.moc widgetcanvas.moc fontchooser.moc \
		kresourceman.moc

backgnd.cpp:backgnd.moc
backgnd.moc: backgnd.h
	$(MOC) backgnd.h -o backgnd.moc

display.cpp:display.moc
display.moc: display.h
	$(MOC) display.h -o display.moc

colorscm.cpp:colorscm.moc
colorscm.moc: colorscm.h
	$(MOC) colorscm.h -o colorscm.moc

savescm.cpp:savescm.moc
savescm.moc: savescm.h
	$(MOC) savescm.h -o savescm.moc

scrnsave.cpp:scrnsave.moc
scrnsave.moc: scrnsave.h
	$(MOC) scrnsave.h -o scrnsave.moc

general.cpp:general.moc
general.moc: general.h
	$(MOC) general.h -o general.moc
	
widgetcanvas.cpp:widgetcanvas.moc
widgetcanvas.moc: widgetcanvas.h
	$(MOC) widgetcanvas.h -o widgetcanvas.moc
	
fontchooser.cpp:fontchooser.moc
fontchooser.moc: fontchooser.h
	$(MOC) fontchooser.h -o fontchooser.moc
	
kresourceman.cpp:kresourceman.moc
kresourceman.moc: kresourceman.h
	$(MOC) kresourceman.h -o kresourceman.moc


install-data-local: 
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) mini-background.xpm $(kde_minidir)/background.xpm
	$(INSTALL_DATA) mini-colors.xpm $(kde_minidir)/colors.xpm
	$(INSTALL_DATA) mini-screensaver.xpm $(kde_minidir)/screensaver.xpm
	$(INSTALL_DATA) mini-style.xpm $(kde_minidir)/style.xpm
	$(mkinstalldirs) $(kde_datadir)/kdisplay/pics
	$(INSTALL_DATA) monitor.xpm $(kde_datadir)/kdisplay/pics
	$(mkinstalldirs) $(kde_icondir)
	$(INSTALL_DATA) background.xpm $(kde_icondir)/background.xpm
	$(INSTALL_DATA) palette.xpm $(kde_icondir)/palette.xpm

uninstall-local:
	rm -f $(kde_minidir)/background.xpm
	rm -f $(kde_minidir)/colors.xpm
	rm -f $(kde_minidir)/screensaver.xpm
	rm -f $(kde_minidir)/style.xpm
	rm -f $(kde_datadir)/kdisplay/pics/monitor.xpm
	rm -f $(kde_icondir)/background.xpm
	rm -f $(kde_icondir)/palette.xpm

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x ../../../kdelibs/po/kde.pot $(kcmdisplay_SOURCES) && mv messages.po ../../po/kdisplay.pot
