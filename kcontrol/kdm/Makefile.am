INCLUDES = $(all_includes) 
LDADD = $(all_libraries) -lkfm -ljpeg -lkdeui -lkdecore -lqt -lXext -lX11 $(LIBSOCKET) $(KDE_RPATH)
SUBDIRS = doc pics applnk

bin_PROGRAMS = kdmconfig

kdmconfig_SOURCES = main.cpp jpeg.cpp utils.cpp kdm-appear.cpp kdm-font.cpp kdm-bgnd.cpp kdm-users.cpp kdm-sess.cpp
kdmconfig_LDADD = $(LDADD)
BUILT_SOURCED =  kdm-appear.moc kdm-font.moc kdm-bgnd.moc kdm-users.moc kdm-sess.moc

kdm-appear.cpp: kdm-appear.moc
kdm-appear.moc: kdm-appear.h
	$(MOC) kdm-appear.h -o kdm-appear.moc

kdm-font.cpp: kdm-font.moc
kdm-font.moc: kdm-font.h
	$(MOC) kdm-font.h -o kdm-font.moc

kdm-bgnd.cpp: kdm-bgnd.moc
kdm-bgnd.moc: kdm-bgnd.h
	$(MOC) kdm-bgnd.h -o kdm-bgnd.moc

kdm-users.cpp: kdm-users.moc
kdm-users.moc: kdm-users.h
	$(MOC) kdm-users.h -o kdm-users.moc

kdm-sess.cpp: kdm-sess.moc
kdm-sess.moc: kdm-sess.h
	$(MOC) kdm-sess.h -o kdm-sess.moc

DISTCLEANFILES = $(BUILT_SOURCES)

messages:
	$(XGETTEXT) -C -ktranslate -x ../../../kdelibs/po/kde.pot $(kdmconfig_SOURCES) && mv messages.po ../../po/kdmconfig.pot
