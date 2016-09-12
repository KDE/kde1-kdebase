# $Id: Makefile.am,v 1.27 1999/01/19 13:43:49 kulow Exp $
# kdebase/kcontrol/display

INCLUDES = $(all_includes) 
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD =  $(LIB_KFILE) $(LIB_KIMGIO) 

SUBDIRS = kcs ad pics

bin_PROGRAMS = kcmdisplay

kcmdisplay_SOURCES = backgnd.cpp display.cpp \
		colorscm.cpp savescm.cpp scrnsave.cpp \
		general.cpp main.cpp widgetcanvas.cpp \
		fontchooser.cpp kresourceman.cpp

METASOURCES = backgnd.moc display.moc colorscm.moc savescm.moc \
		scrnsave.moc general.moc widgetcanvas.moc fontchooser.moc \
		kresourceman.moc

noinst_HEADERS = backgnd.h display.h colorscm.h savescm.h scrnsave.h general.h widgetcanvas.h fontchooser.h kresourceman.h

EXTRA_DIST = README

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x ../../../kdelibs/po/kde.pot $(kcmdisplay_SOURCES) && mv messages.po ../../po/kcmdisplay.pot
