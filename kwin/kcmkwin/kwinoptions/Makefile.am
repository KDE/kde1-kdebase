# $Id$
# kdebase/kcontrol/kwm

SUBDIRS = pics

INCLUDES = $(all_includes) 
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD = $(LIB_KDEUI) 

bin_PROGRAMS = kcmkwm

kcmkwm_SOURCES = main.cpp titlebar.cpp windows.cpp desktop.cpp \
		mouse.cpp advanced.cpp
BUILT_SOURCES = titlebar.moc windows.moc desktop.moc mouse.moc advanced.moc
noinst_HEADERS = windows.h desktop.h titlebar.h geom.h mouse.h advanced.h

windows.cpp: windows.moc
windows.moc: windows.h
	$(MOC) windows.h -o windows.moc


desktop.cpp: desktop.moc
desktop.moc: desktop.h
	$(MOC) desktop.h -o desktop.moc

titlebar.cpp: titlebar.moc
titlebar.moc: titlebar.h
	$(MOC) titlebar.h -o titlebar.moc

mouse.cpp: mouse.moc
mouse.moc: mouse.h
	$(MOC) mouse.h -o mouse.moc

advanced.cpp: advanced.moc
advanced.moc: advanced.h
	$(MOC) advanced.h -o advanced.moc

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(kcmkwm_SOURCES) && mv messages.po ../../po/kcmkwm.pot

DISTCLEANFILES = $(BUILT_SOURCES)
