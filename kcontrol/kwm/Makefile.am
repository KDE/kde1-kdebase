# $Id: Makefile.am,v 1.19 1999/01/18 12:46:26 kulow Exp $
# kdebase/kcontrol/kwm

SUBDIRS = pics

INCLUDES = $(all_includes) 
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD = $(LIB_KDEUI) 

bin_PROGRAMS = kcmkwm

kcmkwm_SOURCES = main.cpp titlebar.cpp windows.cpp desktop.cpp \
		mouse.cpp advanced.cpp
METASOURCES = titlebar.moc windows.moc desktop.moc mouse.moc advanced.moc
noinst_HEADERS = windows.h desktop.h titlebar.h geom.h mouse.h advanced.h

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(kcmkwm_SOURCES) && mv messages.po ../../po/kcmkwm.pot

