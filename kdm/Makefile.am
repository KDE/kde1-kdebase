
SUBDIRS		= config pics @xdmconfigsubdir@

# System depended vars:
MOC	 = @MOC@

# Extra lib. Might be -lpam -dl or -lshadow or nonthing
PASSWDLIB=@PASSWDLIB@

INCLUDES = -I.. @all_includes@

#CFLAGS   = -Wall -Wno-implicit -Wno-return-type
CFLAGS   += -Wno-implicit -Wno-return-type 
CXXFLAGS += -DHAVE_CONFIG_H

bin_PROGRAMS = kdm kdmdesktop

# Libraries:
LDADD  = @all_libraries@ -lkdecore -lqt -lXext\
           -lX11 -lXau -lXdmcp -lm $(PASSWDLIB) @LIBSOCKET@ @LIBCRYPT@

# For kdmdesktop
kdmdesktop_LDADD = @all_libraries@ -lkdecore -lqt -lm -lgif @LIBJPEG@ -lXext -lX11 $(LDADD)

.h.moc:
	$(MOC) -o $@ $<

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
	gif.cpp \
	jpeg.cpp

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



