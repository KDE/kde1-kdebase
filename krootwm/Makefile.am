####### Fiddle here

INCLUDES = $(all_includes)
LDADD   = $(all_libraries) -lkfm -lkdeui -lkdecore -lqt -lXext -lX11 $(LIBSOCKET) $(KDE_RPATH)

#######	Files

bin_PROGRAMS	= krootwm
krootwm_SOURCES	= krootwm.C
BUILTSOURCES =	krootwm.moc 
DISTCLEANFILES = $(BUILTSOURCES)

krootwm.C:krootwm.moc
krootwm.moc: krootwm.h
	$(MOC) krootwm.h -o krootwm.moc

pmenu.C:pmenu.moc
pmenu.moc: pmenu.h
	$(MOC) pmenu.h -o pmenu.moc

messages:
	$(XGETTEXT) -C -ktranslate -x../../kdelibs/po/kde.pot $(krootwm_SOURCES) && mv messages.po ../po/krootwm.pot

