INCLUDES= $(all_includes)

####### Files


bin_PROGRAMS = 	kcmkeys
kcmkeys_SOURCES = main.cpp standard.cpp global.cpp savescm.cpp 
noinst_HEADERS =  standard.h global.h savescm.h
kcmkeys_LDADD = -lkfm -lkdeui -lkdecore -lqt -lXext -lX11 $(LIBSOCKET)
kcmkeys_LDFLAGS = $(all_libraries) $(KDE_RPATH)

BUILTSOURCES = standard.moc savescm.moc global.moc 
DISTCLEANFILES = standard.moc global.moc

savescm.cpp:savescm.moc
savescm.moc: savescm.h
	$(MOC) savescm.h -o savescm.moc

global.cpp:global.moc
global.moc: global.h
	$(MOC) global.h -o global.moc


standard.cpp:standard.moc
standard.moc: standard.h
	$(MOC) standard.h -o standard.moc


install-data-local: 


uninstall-local:

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(kcmkeys_SOURCES) ../../kwm/kwmbindings.cpp && mv messages.po ../../po/kcmkeys.pot
