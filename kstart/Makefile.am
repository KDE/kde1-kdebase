####### Fiddle here

INCLUDES = $(all_includes)
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD   =       $(LIB_KDEUI)

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
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(kstart_SOURCES) && mv messages.po ../po/kstart.pot


