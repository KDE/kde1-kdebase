# $Id$
# kdebase/kcontrol/display

INCLUDES = $(all_includes) 
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD =  $(LIB_KFILE) $(LIB_KIMGIO) 

DISTCLEANFILES = $(BUILT_SOURCES)

SUBDIRS = kcs ad pics

bin_PROGRAMS = kcmdisplay

kcmdisplay_SOURCES = backgnd.cpp display.cpp \
		colorscm.cpp savescm.cpp scrnsave.cpp \
		general.cpp main.cpp widgetcanvas.cpp \
		fontchooser.cpp kresourceman.cpp

BUILT_SOURCES = backgnd.moc display.moc colorscm.moc savescm.moc \
		scrnsave.moc general.moc widgetcanvas.moc fontchooser.moc \
		kresourceman.moc

noinst_HEADERS = backgnd.h display.h colorscm.h savescm.h scrnsave.h general.h widgetcanvas.h fontchooser.h kresourceman.h

backgnd.cpp:backgnd.moc
backgnd.moc: backgnd.h
	$(MOC) $(srcdir)/backgnd.h -o backgnd.moc

display.cpp:display.moc
display.moc: display.h
	$(MOC) $(srcdir)/display.h -o display.moc

colorscm.cpp:colorscm.moc
colorscm.moc: colorscm.h
	$(MOC) $(srcdir)/colorscm.h -o colorscm.moc

savescm.cpp:savescm.moc
savescm.moc: savescm.h
	$(MOC) $(srcdir)/savescm.h -o savescm.moc

scrnsave.cpp:scrnsave.moc
scrnsave.moc: scrnsave.h
	$(MOC) $(srcdir)/scrnsave.h -o scrnsave.moc

general.cpp:general.moc
general.moc: general.h
	$(MOC) $(srcdir)/general.h -o general.moc

widgetcanvas.cpp:widgetcanvas.moc
widgetcanvas.moc: widgetcanvas.h
	$(MOC) $(srcdir)/widgetcanvas.h -o widgetcanvas.moc

fontchooser.cpp:fontchooser.moc
fontchooser.moc: fontchooser.h
	$(MOC) $(srcdir)/fontchooser.h -o fontchooser.moc

kresourceman.cpp:kresourceman.moc
kresourceman.moc: kresourceman.h
	$(MOC) $(srcdir)/kresourceman.h -o kresourceman.moc

EXTRA_DIST = README

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x ../../../kdelibs/po/kde.pot $(kcmdisplay_SOURCES) && mv messages.po ../../po/kcmdisplay.pot
