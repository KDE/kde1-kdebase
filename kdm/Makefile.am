# $Id$

SUBDIRS	= config pics $(xdmconfigsubdir)

# Extra lib. Might be -lpam -dl or -lshadow or nonthing
PASSLIB= $(PASSWDLIB) $(PAMLIBS)

INCLUDES = $(all_includes)

#CFLAGS   = -Wall -Wno-implicit -Wno-return-type
CFLAGS   += -DHAVE_CONFIG_H
CXXFLAGS += -DHAVE_CONFIG_H

bin_PROGRAMS = kdm kdmdesktop

# Libraries:
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD  = $(LIB_KIMGIO) $(LIB_KDECORE) -lXau $(LIBXDMCP) $(PASSLIB) $(LIB_LIBS) \
	$(LIBSOCKET) $(LIBCRYPT) $(LIBUCB) $(LIBUTIL) 

# For kdmdesktop
kdmdesktop_LDADD = $(LIB_KIMGIO) $(LIB_KDECORE)

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
	kdmdesktop.cpp

noinst_HEADERS = kdmconfig.h kdmview.h kdm-config.h kdmshutdown.h kfdialog.h dm.h kgreeter.h greet.h

METASOURCES = kfdialog.moc kdmview.moc kdmshutdown.moc kgreeter.moc

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(kdm_SOURCES) $(kdmdesktop_SOURCES) && mv messages.po ../po/kdm.pot

