#
# Makefile.am for kscreensave.   
#

INCLUDES = $(all_includes)
LDADD = $(all_libraries) -lkdeui -lkdecore -lqt -lXext -lX11 -lm $(PAMLIBS) \
	$(LIBCRYPT) $(LIBSOCKET) $(KDE_RPATH)
DISTCLEANFILES = $(BUILT_SOURCES)

# for shadow passwords override the value
INSTALL_PROGRAM = @INSTALL@ $(SETUIDFLAGS)

bin_PROGRAMS = kpolygon.kss kswarm.kss klissie.kss kbanner.kss \
	klines.kss kblankscrn.kss krock.kss kflame.kss klaser.kss \
	kpyro.kss khop.kss $(XPM_SAVERS) $(GL_SAVERS) kslip.kss \
	kforest.kss klock kblob.kss kbouboule.kss kscience.kss

EXTRA_PROGRAMS = kbat.kss kmorph3d.kss kspace.kss kpipes.kss

EXTRA_DIST = kscreensaver.xpm mini-kscreensaver.xpm kscience.gif kscience-small.gif

BASE_SRCS = main.cpp xautolock.cpp saver.cpp passwd.cpp xlock.cpp \
	    helpers.cpp

SUBDIRS = pixmaps bitmaps

main.cpp: main.moc
saver.cpp: saver.moc

kgroupbox.cpp: kgroupbox.moc

kpolygon_kss_SOURCES = $(BASE_SRCS) polygon.cpp
polygon.cpp: polygon.moc

kswarm_kss_SOURCES = $(BASE_SRCS) swarm.cpp
swarm.cpp: swarm.moc

klissie_kss_SOURCES = $(BASE_SRCS) lissie.cpp
lissie.cpp: lissie.moc

kbanner_kss_SOURCES = $(BASE_SRCS) banner.cpp  kgroupbox.cpp
banner.cpp: banner.moc

kblankscrn_kss_SOURCES = $(BASE_SRCS) blankscrn.cpp
blankscrn.cpp: blankscrn.moc

kbat_kss_SOURCES = $(BASE_SRCS) bat.cpp
kbat_kss_LDADD = $(LDADD) $(XPMLIB)
bat.cpp: bat.moc

krock_kss_SOURCES = $(BASE_SRCS) rock.cpp
rock.cpp: rock.moc

kslip_kss_SOURCES = $(BASE_SRCS) slip.cpp
slip.cpp: slip.moc

kpyro_kss_SOURCES = $(BASE_SRCS) pyro.cpp
pyro.cpp: pyro.moc

klines_kss_SOURCES = $(BASE_SRCS) lines.cpp
lines.cpp: lines.moc

klaser_kss_SOURCES = $(BASE_SRCS) laser.cpp
laser.cpp: laser.moc

khop_kss_SOURCES = $(BASE_SRCS) hop.cpp
hop.cpp: hop.moc

kflame_kss_SOURCES = $(BASE_SRCS) flame.cpp
flame.cpp: flame.moc

kforest_kss_SOURCES = $(BASE_SRCS) forest.cpp
forest.cpp: forest.moc

kblob_kss_SOURCES = $(BASE_SRCS) blob.cpp
blob.cpp: blob.moc

kbouboule_kss_SOURCES = $(BASE_SRCS) bouboule.cpp
bouboule.cpp: bouboule.moc

kmorph3d_kss_SOURCES = $(BASE_SRCS) morph3d.cpp
kmorph3d_kss_LDADD = $(LDADD) $(GLLIB)
morph3d.cpp: morph3d.moc

kpipes_kss_SOURCES = $(BASE_SRCS) pipes.cpp
kpipes_kss_LDADD = $(LDADD) $(GLLIB)
pipes.cpp: pipes.moc

kspace_kss_SOURCES = $(BASE_SRCS) space.cpp
kspace_kss_LDADD = $(LDADD) $(GLLIB)
space.cpp: space.moc

kscience_kss_SOURCES = $(BASE_SRCS) science.cpp
kscience_kss_LDADD = $(LDADD) -lm
science.cpp: science.moc

klock_SOURCES = klock.cpp kscreensave.cpp


BUILT_SOURCES = main.moc saver.moc polygon.moc banner.moc\
		blankscrn.moc rock.moc flame.moc lines.moc laser.moc\
		pyro.moc hop.moc forest.moc swarm.moc bat.moc slip.moc\
		lissie.moc morph3d.moc blob.moc space.moc bouboule.moc \
		kgroupbox.moc science.moc pipes.moc

noinst_HEADERS = saver.h polygon.h banner.h\
		blankscrn.h kscreensave.h xlock.h rock.h flame.h lines.h\
		laser.h pyro.h hop.h forest.h swarm.h bat.h slip.h\
		lissie.h blob.h space.h bouboule.h  kgroupbox.h science.h main.h \
		xautolock.h xlockmore.h mode.h helpers.h morph3d.h pipes.h

messages:
	$(XGETTEXT) -C -ktranslate -ki18n *.cpp -x ../../kdelibs/po/kde.pot && mv messages.po ../po/klock.pot

########## Meta objects ##########

main.moc: main.h
	$(MOC) main.h -o main.moc

saver.moc: saver.h
	$(MOC) saver.h -o saver.moc

polygon.moc: polygon.h
	$(MOC) polygon.h -o polygon.moc

banner.moc: banner.h
	$(MOC) banner.h -o banner.moc

blankscrn.moc: blankscrn.h
	$(MOC) blankscrn.h -o blankscrn.moc

rock.moc: rock.h
	$(MOC) rock.h -o rock.moc

flame.moc: flame.h
	$(MOC) flame.h -o flame.moc

lines.moc: lines.h
	$(MOC) lines.h -o lines.moc

laser.moc: laser.h
	$(MOC) laser.h -o laser.moc

pyro.moc: pyro.h
	$(MOC) pyro.h -o pyro.moc

hop.moc: hop.h
	$(MOC) hop.h -o hop.moc

forest.moc: forest.h
	$(MOC) forest.h -o forest.moc

swarm.moc: swarm.h
	$(MOC) swarm.h -o swarm.moc

space.moc: space.h
	$(MOC) space.h -o space.moc

bat.moc: bat.h
	$(MOC) bat.h -o bat.moc

slip.moc: slip.h
	$(MOC) slip.h -o slip.moc

lissie.moc: lissie.h
	$(MOC) lissie.h -o lissie.moc

blob.moc: blob.h
	$(MOC) blob.h -o blob.moc

bouboule.moc: bouboule.h
	$(MOC) bouboule.h -o bouboule.moc

morph3d.moc: morph3d.h
	$(MOC) morph3d.h -o morph3d.moc

pipes.cpp: pipes.h pipes.moc

pipes.moc: pipes.h
	$(MOC) pipes.h -o pipes.moc

science.moc: science.h
	$(MOC) science.h -o science.moc

kgroupbox.moc: kgroupbox.h
	$(MOC) kgroupbox.h -o kgroupbox.moc

install-data-local: 
	$(mkinstalldirs) $(kde_icondir)
	$(INSTALL_DATA) kscreensaver.xpm $(kde_icondir)/kscreensaver.xpm
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) mini-kscreensaver.xpm $(kde_minidir)/kscreensaver.xpm
	$(mkinstalldirs) $(kde_datadir)/kscreensaver
	$(INSTALL_DATA) kscience.gif $(kde_datadir)/kscreensaver/kscience.gif
	$(mkinstalldirs) $(kde_datadir)/kscreensaver
	$(INSTALL_DATA) kscience-small.gif $(kde_datadir)/kscreensaver/kscience-small.gif

uninstall-local:
	rm -f $(kde_icondir)/kscreensaver.xpm
	rm -f $(kde_minidir)/kscreensaver.xpm
	rm -f $(kde_datadir)/kscreensaver/kscience.gif
	rm -f $(kde_datadir)/kscreensaver/kscience-small.gif

