INCLUDES= $(all_includes)

####### Files

bin_PROGRAMS = 	kcmkeys
kcmkeys_SOURCES = main.cpp keyconfig.cpp savescm.cpp 
noinst_HEADERS =  keyconfig.h savescm.h
kcmkeys_LDADD =   $(LIB_KFM) $(LIB_KDEUI)
kcmkeys_LDFLAGS = $(all_libraries) $(KDE_RPATH)

#BUILTSOURCES = standard.moc savescm.moc global.moc 
#DISTCLEANFILES = standard.moc global.moc
BUILTSOURCES = keyconfig.moc savescm.moc  
DISTCLEANFILES = keyconfig.moc 

savescm.cpp:savescm.moc
savescm.moc: savescm.h
	$(MOC) $(srcdir)/savescm.h -o savescm.moc

keyconfig.cpp:keyconfig.moc
keyconfig.moc: keyconfig.h
	$(MOC) $(srcdir)/keyconfig.h -o keyconfig.moc

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(kcmkeys_SOURCES) ../../kwm/kwmbindings.cpp ./stdbindings.cpp && mv messages.po ../../po/kcmkeys.pot












