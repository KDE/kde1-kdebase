INCLUDES= $(all_includes)

####### Files

bin_PROGRAMS = 	kcmkeys
kcmkeys_SOURCES = main.cpp keyconfig.cpp savescm.cpp 
noinst_HEADERS =  keyconfig.h savescm.h
kcmkeys_LDADD =   $(LIB_KFM) $(LIB_KDEUI)
kcmkeys_LDFLAGS = $(all_libraries) $(KDE_RPATH)

METASOURCES = keyconfig.moc savescm.moc  

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(kcmkeys_SOURCES) ../../kwm/kwmbindings.cpp ./stdbindings.cpp && mv messages.po ../../po/kcmkeys.pot

$(srcdir)/keyconfig.cpp: stdbindings.cpp

