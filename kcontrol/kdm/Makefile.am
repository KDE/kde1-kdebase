INCLUDES = $(all_includes) 
LDADD = $(all_libraries) -lkfile -lkfm -lkdeui -lkdecore -lqt $(KIMGIO_LIBS) -lkimgio -lXext -lX11 $(LIBSOCKET) $(KDE_RPATH)
SUBDIRS = doc pics 

bin_PROGRAMS = kdmconfig

kdmconfig_SOURCES = main.cpp utils.cpp kdm-appear.cpp kdm-font.cpp kdm-bgnd.cpp \
	kdm-users.cpp kdm-sess.cpp klangcombo.cpp kdropsite.cpp
kdmconfig_LDADD = $(LDADD)
#kdmconfig_LDFLAGS = -g
BUILT_SOURCES =  kdm-appear.moc kdm-font.moc kdm-bgnd.moc kdm-users.moc kdm-sess.moc \
	klangcombo.moc
noinst_HEADERS = kdm-appear.h kdm-font.h kdm-bgnd.h kdm-users.h kdm-sess.h klangcombo.h \
	utils.h kdropsite.h

kdm-appear.cpp: kdm-appear.moc
kdm-appear.moc: kdm-appear.h
	$(MOC) kdm-appear.h -o kdm-appear.moc

klangcombo.cpp: klangcombo.moc
klangcombo.moc: klangcombo.h
	$(MOC) klangcombo.h -o klangcombo.moc

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

kdropsite.cpp: kdropsite.moc
kdropsite.moc: kdropsite.h
	$(MOC) kdropsite.h -o kdropsite.moc

DISTCLEANFILES = $(BUILT_SOURCES)

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(kdmconfig_SOURCES) && mv messages.po ../../po/kdmconfig.pot
