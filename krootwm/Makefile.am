####### Fiddle here

INCLUDES = $(all_includes)
LDFLAGS =  $(all_libraries) $(KDE_RPATH)
LDADD   =  $(LIB_KFM) $(LIB_KDEUI) 

#######	Files

bin_PROGRAMS	= krootwm
krootwm_SOURCES	= krootwm.C
METASOURCES =	krootwm.moc 
DISTCLEANFILES = $(BUILTSOURCES)
noinst_HEADERS = krootwm.h version.h

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(krootwm_SOURCES) && mv messages.po ../po/krootwm.pot

