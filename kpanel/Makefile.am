####### kpanel Makefile.am
## written by Stephan Kulow using the old Makefile by Matthias Ettrich
##

MOC	= 	@MOC@
INCLUDES=	@all_includes@ 
CLEANFILES = 	$(BUILT_SOURCES)

bin_PROGRAMS = kpanel
kpanel_LDADD =	@all_libraries@ -lkfm -lkdeui -lkdecore -lqt -lXext -lX11 @LIBSOCKET@
kpanel_SOURCES = kpanel.C props.C layout.C tools.C main.C events.C pmenu.C
noinst_HEADERS = kpanel.h pmenu.h
BUILT_SOURCES = kpanel.moc pmenu.moc
SUBDIRS = pics

main.C: kpanel.moc
pmenu.C: pmenu.moc

kpanel.moc: kpanel.h
	$(MOC) kpanel.h -o kpanel.moc
pmenu.moc: pmenu.h
	$(MOC) pmenu.h -o pmenu.moc

messages:
	@XGETTEXT@ -C --keyword=translate -x ../../kdelibs/po/kde.po $(kpanel_SOURCES)
