####### Fiddle here

INCLUDES = $(all_includes)
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD   = -lkfm -lkdeui -lkdecore -lqt -lXext -lX11 $(LIBSOCKET) 

#######	Files

bin_PROGRAMS	= krootwm
krootwm_SOURCES	= krootwm.C
BUILTSOURCES =	krootwm.moc 
DISTCLEANFILES = $(BUILTSOURCES)
noinst_HEADERS = krootwm.h version.h

krootwm.C:krootwm.moc
krootwm.moc: krootwm.h
	$(MOC) krootwm.h -o krootwm.moc

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(krootwm_SOURCES) && mv messages.po ../po/krootwm.pot

