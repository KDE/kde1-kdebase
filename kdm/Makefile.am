
SUBDIRS		= config doc pics @xdmconfigsubdir@

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
kdm_LDADD  = @all_libraries@ -lkdecore -lqt -lXext\
           -lX11 -lXau -lXdmcp -lm $(PASSWDLIB) @LIBSOCKET@

# For kdmdesktop
kdedesktop_LDADD = @all_libraries@ -lkdecore -lqt -lm -lgif -ljpeg -lXext -lX11

.h.moc:
	$(MOC) -o $@ $<

# Sources for xdm (both C and C++):
kdm_SOURCES = access.c \
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

kdmdesktop_SOURCES = kdmconfig.cpp \
	     kdmshutdown.cpp \
	     kdmview.cpp \
	     kfdialog.cpp \
	     kgreeter.cpp

# Sources for kdmdesktop:
KDMDSOURCES = kdmdesktop.cpp gif.cpp jpeg.cpp

MOCSRC = kfdialog.moc kdmview.moc kdmshutdown.moc kgreeter.moc

# Objects:

KDMOBJECTS = $(CSOURCES:.c=.o) $(CXXSOURCES:.cpp=.o)
KDMDOBJECTS = $(KDMDSOURCES:.cpp=.o)

# Targets:

all: 
	@for subdir in $(SUBDIRS); do \
	echo "Making in $$subdir"; \
	(cd $$subdir && $(MAKE)) \
	|| case "$(MFLAGS)" in *k*) fail=yes;; *) exit 1;; esac; \
	done && test -z "$$fail"
	@echo ""

Makefile: Makefile.in
	cd .. && CONFIG_FILES=kdm/$@ CONFIG_HEADERS= $(SHELL) ./config.status  

kdm: $(MOCSRC) $(KDMOBJECTS)
	$(CXX) -o kdm $(KDMOBJECTS) $(KDMLIBS) 

kdmdesktop: $(KDMDOBJECTS)
	$(CXX) -o kdmdesktop $(KDMDOBJECTS) $(KDMDLIBS)

clean:
	$(RM) *.o *~ *.moc kdm kdmdesktop
	@for subdir in $(SUBDIRS); do \
	echo "Making clean in $$subdir"; \
	(cd $$subdir && $(MAKE) clean) \
	|| case "$(MFLAGS)" in *k*) fail=yes;; *) exit 1;; esac; \
	done && test -z "$$fail"
	@echo ""

install: kdm kdmdesktop
	$(mkinstalldirs) $(BINDIR)
	$(INSTALL) kdm $(BINDIR)
	$(INSTALL) kdmdesktop $(BINDIR)	
	@for subdir in $(SUBDIRS); do \
	echo "Making install in $$subdir"; \
	(cd $$subdir && $(MAKE) install) \
	|| case "$(MFLAGS)" in *k*) fail=yes;; *) exit 1;; esac; \
	done && test -z "$$fail"
	@echo ""
