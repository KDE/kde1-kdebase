####### Fiddle here

INCLUDES = $(all_includes)
LDADD   = $(all_libraries) -lkdeui -lkdecore -lqt -lXext -lX11 $(LIBSOCKET) $(KDE_RPATH)

#######	Files

bin_PROGRAMS	= kstartondesk
kstartondesk_SOURCES	= kstartondesk.C
BUILTSOURCES =	kstartondesk.moc 
DISTCLEANFILES = $(BUILTSOURCES)

kstartondesk.C:kstartondesk.moc
kstartondesk.moc: kstartondesk.h
	$(MOC) kstartondesk.h -o kstartondesk.moc


messages:
	$(XGETTEXT) -C -ktranslate -x../../kdelibs/po/kde.pot $(kstartondesk_SOURCES) && mv messages.po ../po/kstartondesk.pot

