
INCLUDES = $(all_includes) 
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD =      $(LIB_KIMGIO)    $(LIB_KFILE) 
SUBDIRS = doc pics 

bin_PROGRAMS = kdmconfig

kdmconfig_SOURCES = main.cpp utils.cpp kdm-appear.cpp kdm-font.cpp kdm-bgnd.cpp \
	kdm-users.cpp kdm-sess.cpp klangcombo.cpp kdropsite.cpp
kdmconfig_LDADD = $(LDADD)
#kdmconfig_LDFLAGS = -g
METASOURCES =  kdm-appear.moc kdm-font.moc kdm-bgnd.moc kdm-users.moc kdm-sess.moc \
	klangcombo.moc kdropsite.moc
noinst_HEADERS = kdm-appear.h kdm-font.h kdm-bgnd.h kdm-users.h kdm-sess.h klangcombo.h \
	utils.h kdropsite.h

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(kdmconfig_SOURCES) && mv messages.po ../../po/kdmconfig.pot
