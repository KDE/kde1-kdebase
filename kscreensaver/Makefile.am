# $Id$
# Makefile.am for kscreensave.   
#

INCLUDES = $(all_includes)
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD = $(PAMLIBS) $(LIBCRYPT) $(LIB_KDEUI) $(PASSWDLIB) -lm 

bin_PROGRAMS = kpolygon.kss kswarm.kss klissie.kss kbanner.kss \
	klines.kss kblankscrn.kss krock.kss kflame.kss klaser.kss \
	kpyro.kss khop.kss $(XPM_SAVERS) $(GL_SAVERS) kslip.kss \
	kforest.kss klock kblob.kss kbouboule.kss kscience.kss \
	kattraction.kss kslidescreen.kss kbsod.kss

EXTRA_PROGRAMS = kbat.kss kmorph3d.kss kspace.kss kpipes.kss kmatrix.kss

EXTRA_DIST = kscreensaver.xpm mini-kscreensaver.xpm kscience.gif kscience-small.gif kscreensave-0.3.3.lsm kscreensaver.pam

BASE_SRCS = main.cpp xautolock.cpp saver.cpp passwd.cpp xlock.cpp \
	    helpers.cpp xs_colors.c xs_hsv.c xs_visual.c

SUBDIRS = pixmaps bitmaps

kmatrix_kss_SOURCES = $(BASE_SRCS) matrix.cpp
kmatrix_kss_LDADD= $(LDADD) $(XPMLIB)

kbsod_kss_SOURCES = $(BASE_SRCS) bsod.cpp
kbsod_kss_LDADD= $(LDADD) $(XPMLIB)

kpolygon_kss_SOURCES = $(BASE_SRCS) polygon.cpp

kswarm_kss_SOURCES = $(BASE_SRCS) swarm.cpp

klissie_kss_SOURCES = $(BASE_SRCS) lissie.cpp

kbanner_kss_SOURCES = $(BASE_SRCS) banner.cpp  kgroupbox.cpp

kblankscrn_kss_SOURCES = $(BASE_SRCS) blankscrn.cpp

kbat_kss_SOURCES = $(BASE_SRCS) bat.cpp
kbat_kss_LDADD = $(LDADD) $(XPMLIB)

krock_kss_SOURCES = $(BASE_SRCS) rock.cpp

kslip_kss_SOURCES = $(BASE_SRCS) slip.cpp

kattraction_kss_SOURCES = $(BASE_SRCS) attraction.cpp

kslidescreen_kss_SOURCES = $(BASE_SRCS) slidescreen.cpp

kpyro_kss_SOURCES = $(BASE_SRCS) pyro.cpp

klines_kss_SOURCES = $(BASE_SRCS) lines.cpp

klaser_kss_SOURCES = $(BASE_SRCS) laser.cpp

khop_kss_SOURCES = $(BASE_SRCS) hop.cpp
kflame_kss_SOURCES = $(BASE_SRCS) flame.cpp
kforest_kss_SOURCES = $(BASE_SRCS) forest.cpp
kblob_kss_SOURCES = $(BASE_SRCS) blob.cpp
kbouboule_kss_SOURCES = $(BASE_SRCS) bouboule.cpp
kmorph3d_kss_SOURCES = $(BASE_SRCS) morph3d.cpp
kmorph3d_kss_LDADD = $(LDADD) $(GLLIB)
kpipes_kss_SOURCES = $(BASE_SRCS) pipes.cpp
kpipes_kss_LDADD = $(LDADD) $(GLLIB)
kspace_kss_SOURCES = $(BASE_SRCS) space.cpp
kspace_kss_LDADD = $(LDADD) $(GLLIB)
kscience_kss_SOURCES = $(BASE_SRCS) science.cpp
kscience_kss_LDADD = $(LDADD) -lm
klock_SOURCES = klock.cpp kscreensave.cpp


METASOURCES = main.moc saver.moc polygon.moc banner.moc\
		blankscrn.moc rock.moc flame.moc lines.moc laser.moc\
		pyro.moc hop.moc forest.moc swarm.moc bat.moc slip.moc\
		lissie.moc morph3d.moc blob.moc space.moc bouboule.moc \
		kgroupbox.moc science.moc pipes.moc attraction.moc \
        slidescreen.moc bsod.moc matrix.moc

noinst_HEADERS = saver.h polygon.h banner.h\
		blankscrn.h kscreensave.h xlock.h rock.h flame.h lines.h\
		laser.h pyro.h hop.h forest.h swarm.h bat.h slip.h\
		lissie.h blob.h space.h bouboule.h  kgroupbox.h science.h main.h \
		xautolock.h xlockmore.h mode.h helpers.h morph3d.h pipes.h \
		attraction.h xs_colors.h xs_hsv.h xs_yarandom.h xs_utils.h \
        slidescreen.h amiga.xpm atari.xbm bsod.h mac.xbm xs_visual.h \
        matrix.h

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot *.cpp *.h && mv messages.po ../po/klock.pot

########## Meta objects ##########

kde_icon_DATA = kscreensaver.xpm 

data_DATA = kscience.gif kscience-small.gif
datadir = $(kde_datadir)/kscreensaver

install-data-local: 
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) $(srcdir)/mini-kscreensaver.xpm $(kde_minidir)/kscreensaver.xpm

uninstall-local:
	rm -f $(kde_minidir)/kscreensaver.xpm

