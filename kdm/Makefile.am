
SUBDIRS	= config pics $(xdmconfigsubdir)

# Extra lib. Might be -lpam -dl or -lshadow or nonthing
PASSLIB= $(PASSWDLIB) $(PAMLIBS)

INCLUDES = $(all_includes)

#CFLAGS   = -Wall -Wno-implicit -Wno-return-type
CFLAGS   += -DHAVE_CONFIG_H
CXXFLAGS += -DHAVE_CONFIG_H

bin_PROGRAMS = kdm kdmdesktop

# Libraries:
LDADD  = $(all_libraries) -lkdecore -lqt -lXext\
           -lXau -lXdmcp -lX11 -lm $(PASSLIB) $(LIB_LIBS) $(LIBSOCKET) $(LIBCRYPT) $(KDE_RPATH) $(LIBUCB) $(LIBUTIL)

# For kdmdesktop
kdmdesktop_LDADD = $(all_libraries) -lkdecore -lqt -lm $(LIBJPEG) -lXext\
		   -lX11 $(LIBSOCKET) $(KDE_RPATH)

# Sources for xdm (both C and C++):
kdmlib_source = access.c \
           auth.c \
	   choose.c \
	   daemon.c \
	   dm.c \
	   dpylist.c \
	   error.c \
	   file.c \
	   genauth.c \
	   mitauth.c \
	   netaddr.c \
	   policy.c \
	   protodpy.c \
	   reset.c \
	   resource.c \
	   rpcauth.c \
	   server.c \
	   session.c \
	   socket.c \
	   streams.c \
	   util.c \
	   verify.c \
	   xdmauth.c \
	   xdmcp.c

kdm_SOURCES = kdmconfig.cpp \
	kdmshutdown.cpp \
	kdmview.cpp \
	kfdialog.cpp \
	kgreeter.cpp \
	$(kdmlib_source)

kdmdesktop_SOURCES = \
	kdmdesktop.cpp \
	jpeg.cpp

noinst_HEADERS = kdmconfig.h kdmview.h kdm-config.h kdmshutdown.h kfdialog.h dm.h kgreeter.h greet.h

MOCSRC = kfdialog.moc kdmview.moc kdmshutdown.moc kgreeter.moc
DISTCLEANFILES = $(MOCSRC)

kfdialog.cpp: kfdialog.moc
kfdialog.moc: kfdialog.h
	$(MOC) kfdialog.h -o kfdialog.moc

kdmview.cpp: kdmview.moc
kdmview.moc: kdmview.h
	$(MOC) kdmview.h -o kdmview.moc

kdmshutdown.cpp: kdmshutdown.moc
kdmshutdown.moc: kdmshutdown.h
	$(MOC) kdmshutdown.h -o kdmshutdown.moc

kgreeter.cpp: kgreeter.moc
kgreeter.moc: kgreeter.h
	$(MOC) kgreeter.h -o kgreeter.moc

messages:
	$(XGETTEXT) -C -ktranslate -x ../../kdelibs/po/kde.pot $(kdm_SOURCES) $(kdmdesktop_SOURCES) && mv messages.po ../po/kdm.pot

