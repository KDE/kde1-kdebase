####### Fiddle here

INCLUDES = $(all_includes)
LDADD   = $(all_libraries) -lkdeui -lkdecore -lqt -lXext -lX11 $(LIBSOCKET) $(KDE_RPATH)

#######	Files

bin_PROGRAMS	= kstart
kstart_SOURCES	= kstart.C
BUILTSOURCES =	kstart.moc 
DISTCLEANFILES = $(BUILTSOURCES)

noinst_HEADERS = kstart.h version.h

kstart.C:kstart.moc
kstart.moc: kstart.h
	$(MOC) kstart.h -o kstart.moc


messages:
	$(XGETTEXT) -C -ktranslate -x../../kdelibs/po/kde.pot $(kstart_SOURCES) && mv messages.po ../po/kstart.pot

