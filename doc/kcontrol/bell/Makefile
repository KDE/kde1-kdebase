# Generated automatically from Makefile.in by configure.
# Makefile.in generated automatically by automake 1.2a from Makefile.am

# Copyright (C) 1994, 1995, 1996, 1997 Free Software Foundation, Inc.
# This Makefile.in is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.


SHELL = /bin/sh

srcdir = .
top_srcdir = ../../..
prefix = /home/coolo/prod/kde
exec_prefix = ${prefix}

bindir = ${exec_prefix}/bin
sbindir = ${exec_prefix}/sbin
libexecdir = ${exec_prefix}/libexec
sysconfdir = ${prefix}/etc
sharedstatedir = ${prefix}/com
localstatedir = ${prefix}/var
libdir = ${exec_prefix}/lib
infodir = ${prefix}/info
mandir = ${prefix}/man
includedir = ${prefix}/include
oldincludedir = /usr/include

pkgdatadir = $(datadir)/kdebase
pkglibdir = $(libdir)/kdebase
pkgincludedir = $(includedir)/kdebase

top_builddir = ../../..

ACLOCAL = aclocal
AUTOCONF = autoconf
AUTOMAKE = automake
AUTOHEADER = autoheader

INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL}
INSTALL_DATA = ${INSTALL} -m 644
INSTALL_SCRIPT = ${INSTALL_PROGRAM}
transform = s,x,x,

NORMAL_INSTALL = true
PRE_INSTALL = true
POST_INSTALL = true
NORMAL_UNINSTALL = true
PRE_UNINSTALL = true
POST_UNINSTALL = true
build_alias = i486-pc-linux-gnulibc1
build_triplet = i486-pc-linux-gnulibc1
host_alias = i486-pc-linux-gnulibc1
host_triplet = i486-pc-linux-gnulibc1
target_alias = i486-pc-linux-gnulibc1
target_triplet = i486-pc-linux-gnulibc1
CC = gcc
CPP = @CPP@
CXX = g++
GLINC = 
GLLIB = -lMesaGL -lMesaGLU
GL_SAVERS = kmorph3d.kss
GMSGFMT = /usr/bin/msgfmt
KCC_MODULES = -dlpreopen ../sample/libsample.la -dlpreopen ../display/libdisplay.la -dlpreopen ../keyboard/libkeyboard.la -dlpreopen ../memory/libmemory.la -dlpreopen ../mouse/libmouse.la -dlpreopen ../processor/libprocessor.la -dlpreopen ../smbstatus/libsmbstatus.la -dlpreopen ../titlebar/libtitlebar.la -lgif -ljpeg
KDE_INCLUDES = -I/home/coolo/prod/kde/include
KDE_LDFLAGS = -L/home/coolo/prod/kde/lib
KDE_RPATH = -rpath $(kde_libraries) -rpath $(qt_libraries)
LD = /usr/i486-linux/bin/ld
LIBCOMPAT = 
LIBCRYPT = 
LIBDL = @LIBDL@
LIBJPEG = -ljpeg
LIBOBJS = @LIBOBJS@
LIBSOCKET = 
LIBTOOL = $(SHELL) $(top_builddir)/libtool
LIBUCB = 
LN_S = ln -s
MAKEINFO = makeinfo
MOC = /home/coolo/prod/qt-1.30/bin/moc
MSGFMT = /usr/bin/msgfmt
NM = /usr/bin/nm -B
PACKAGE = kdebase
PAMINC = 
PAMLIBPATHS = 
PAMLIBS = 
PASSWDLIB = 
QT_INCLUDES = -I/home/coolo/prod/qt-1.30/include
QT_LDFLAGS = -L/home/coolo/prod/qt-1.30/lib
RANLIB = ranlib
SETUIDFLAGS = 
USE_NLS = yes
VERSION = 0.10
XBINDIR = /usr/X11R6/bin
XGETTEXT = /usr/bin/xgettext
XPMINC = 
XPMLIB = -lXpm
XPM_SAVERS = kbat.kss
X_INCLUDES = -I/usr/X11R6/include
X_LDFLAGS = -L/usr/X11R6/lib
X_SERVER = /usr/X11R6/bin
ac_xdmdir = /usr/X11R6/lib/X11/xdm
all_includes = -I/home/coolo/prod/kde/include -I/home/coolo/prod/qt-1.30/include -I/usr/X11R6/include
all_libraries = -L/home/coolo/prod/kde/lib -L/home/coolo/prod/qt-1.30/lib -L/usr/X11R6/lib
kcc_moduledir = $(prefix)/lib/kcc/modules
kde_appsdir = $(prefix)/share/applnk
kde_bindir = $(exec_prefix)/bin
kde_cgidir = $(exec_prefix)/cgi-bin
kde_confdir = $(prefix)/share/config
kde_datadir = $(prefix)/share/apps
kde_htmldir = $(prefix)/share/doc/HTML
kde_icondir = $(prefix)/share/icons
kde_includes = /home/coolo/prod/kde/include
kde_kccmodules = $(prefix)/lib/kcc/modules
kde_libraries = /home/coolo/prod/kde/lib
kde_locale = $(prefix)/share/locale
kde_mimedir = $(prefix)/share/mimelnk
kde_minidir = $(prefix)/share/icons/mini
kde_partsdir = $(exec_prefix)/parts
kde_toolbardir = $(prefix)/share/toolbar
kde_wallpaperdir = $(prefix)/share/wallpapers
qt_includes = /home/coolo/prod/qt-1.30/include
qt_libraries = /home/coolo/prod/qt-1.30/lib
topdir = /var/home/coolo/prod/kdebase
xdmconfigsubdir = 

data_DATA = bell.html

datadir = $(kde_htmldir)/en/kcontrol/bell
mkinstalldirs = $(SHELL) $(top_srcdir)/mkinstalldirs
CONFIG_HEADER = ../../../config.h
CONFIG_CLEAN_FILES = 
DATA =  $(data_DATA)

DIST_COMMON =  Makefile.am Makefile.in


DISTFILES = $(DIST_COMMON) $(SOURCES) $(HEADERS) $(TEXINFOS) $(EXTRA_DIST)

TAR = tar
GZIP = --best
default: all

.SUFFIXES:
$(srcdir)/Makefile.in: Makefile.am $(top_srcdir)/configure.in $(ACLOCAL_M4) 
	cd $(top_srcdir) && $(AUTOMAKE) --gnu doc/kcontrol/bell/Makefile

Makefile: $(srcdir)/Makefile.in $(top_builddir)/config.status $(BUILT_SOURCES)
	cd $(top_builddir) \
	  && CONFIG_FILES=$(subdir)/$@ CONFIG_HEADERS= $(SHELL) ./config.status


install-dataDATA: $(data_DATA)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(datadir)
	@list='$(data_DATA)'; for p in $$list; do \
	  if test -f $(srcdir)/$$p; then \
	    echo " $(INSTALL_DATA) $(srcdir)/$$p $(datadir)/$$p"; \
	    $(INSTALL_DATA) $(srcdir)/$$p $(datadir)/$$p; \
	  else if test -f $$p; then \
	    echo " $(INSTALL_DATA) $$p $(datadir)/$$p"; \
	    $(INSTALL_DATA) $$p $(datadir)/$$p; \
	  fi; fi; \
	done

uninstall-dataDATA:
	$(NORMAL_UNINSTALL)
	list='$(data_DATA)'; for p in $$list; do \
	  rm -f $(datadir)/$$p; \
	done
tags: TAGS
TAGS:


distdir = $(top_builddir)/$(PACKAGE)-$(VERSION)/$(subdir)

subdir = doc/kcontrol/bell

distdir: $(DISTFILES)
	here=`cd $(top_builddir) && pwd`; \
	top_distdir=`cd $(top_distdir) && pwd`; \
	cd $(top_srcdir) \
	  && $(AUTOMAKE) --include-deps --build-dir=$$here --srcdir-name=$(top_srcdir) --output-dir=$$top_distdir --gnu doc/kcontrol/bell/Makefile
	@for file in $(DISTFILES); do \
	  d=$(srcdir); \
	  test -f $(distdir)/$$file \
	  || ln $$d/$$file $(distdir)/$$file 2> /dev/null \
	  || cp -p $$d/$$file $(distdir)/$$file; \
	done
info:
dvi:
check: all
	$(MAKE)
installcheck:
install-exec: 
	@$(NORMAL_INSTALL)

install-data: install-dataDATA
	@$(NORMAL_INSTALL)

install: install-exec install-data all
	@:

uninstall: uninstall-dataDATA

all: Makefile $(DATA)

install-strip:
	$(MAKE) INSTALL_PROGRAM='$(INSTALL_PROGRAM) -s' INSTALL_SCRIPT='$(INSTALL_PROGRAM)' install
installdirs:
	$(mkinstalldirs)  $(datadir)


mostlyclean-generic:
	test -z "$(MOSTLYCLEANFILES)" || rm -f $(MOSTLYCLEANFILES)

clean-generic:
	test -z "$(CLEANFILES)" || rm -f $(CLEANFILES)

distclean-generic:
	rm -f Makefile $(DISTCLEANFILES)
	rm -f config.cache config.log stamp-h stamp-h[0-9]*
	test -z "$(CONFIG_CLEAN_FILES)" || rm -f $(CONFIG_CLEAN_FILES)

maintainer-clean-generic:
	test -z "$(MAINTAINERCLEANFILES)" || rm -f $(MAINTAINERCLEANFILES)
	test -z "$(BUILT_SOURCES)" || rm -f $(BUILT_SOURCES)
mostlyclean:  mostlyclean-generic

clean:  clean-generic mostlyclean

distclean:  distclean-generic clean
	rm -f config.status
	rm -f libtool

maintainer-clean:  maintainer-clean-generic distclean
	@echo "This command is intended for maintainers to use;"
	@echo "it deletes files that may require special tools to rebuild."

.PHONY: default uninstall-dataDATA install-dataDATA tags distdir info \
dvi installcheck install-exec install-data install uninstall all \
installdirs mostlyclean-generic distclean-generic clean-generic \
maintainer-clean-generic clean mostlyclean distclean maintainer-clean


# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
