INCLUDES = $(all_includes) 
LDADD = $(all_libraries) -lkdeui -lkdecore -lqt -lXext -lX11 -lgif $(LIBJPEG) $(LIBSOCKET) $(KDE_RPATH)

DISTCLEANFILES = $(BUILT_SOURCES)

bin_PROGRAMS = kcmdisplay

kcmdisplay_SOURCES = backgnd.cpp gif.cpp jpeg.cpp display.cpp \
                        colorscm.cpp kwmcom.cpp savescm.cpp scrnsave.cpp \
			general.cpp main.cpp
BUILT_SOURCES = backgnd.moc display.moc colorscm.moc savescm.moc kwmcom.moc \
                scrnsave.moc general.moc

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

kwmcom.cpp:kwmcom.moc
kwmcom.moc: kwmcom.h
	$(MOC) kwmcom.h -o kwmcom.moc

scrnsave.cpp:scrnsave.moc
scrnsave.moc: scrnsave.h
	$(MOC) scrnsave.h -o scrnsave.moc

general.cpp:general.moc
general.moc: general.h
	$(MOC) general.h -o general.moc


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
	$(XGETTEXT) -C -ktranslate -x ../../../kdelibs/po/kde.pot $(kcmdisplay_SOURCES) && mv messages.po ../../po/kdisplay.pot
