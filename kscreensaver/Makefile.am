#
# Makefile.am for kscreensave.   Good Luck
#

INCLUDES = $(all_includes)
LDADD = $(all_libraries) -lkdeui -lkdecore -lqt -lXext -lX11 $(LIBCRYPT) $(LIBSOCKET) $(KDE_RPATH)
DISTCLEANFILES = $(BUILT_SOURCES)

# for shadow passwords override the value
INSTALL = @INSTALL@ $(SETUIDFLAGS)

bin_PROGRAMS = kpolygon.kss kswarm.kss klissie.kss kbanner.kss \
	klines.kss kblankscrn.kss krock.kss kflame.kss klaser.kss \
	kpyro.kss khop.kss $(XPM_SAVERS) $(GL_SAVERS) kslip.kss \
	kforest.kss klock kblob.kss

EXTRA_PROGRAMS = kbat.kss kmorph3d.kss

BASE_SRCS = main.cpp xautolock.cpp saver.cpp passwd.cpp xlock.cpp
main.cpp: main.moc
saver.cpp: saver.moc

kpolygon_kss_SOURCES = $(BASE_SRCS) polygon.cpp
polygon.cpp: polygon.moc

kswarm_kss_SOURCES = $(BASE_SRCS) swarm.cpp
swarm.cpp: swarm.moc

klissie_kss_SOURCES = $(BASE_SRCS) lissie.cpp
lissie.cpp: lissie.moc

kbanner_kss_SOURCES = $(BASE_SRCS) banner.cpp
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

kmorph3d_kss_SOURCES = $(BASE_SRCS) morph3d.cpp
kmorph3d_kss_LDADD = $(LDADD) $(GLLIB)
morph3d.cpp: morph3d.moc

klock_SOURCES = klock.cpp kscreensave.cpp


BUILT_SOURCES = main.moc saver.moc polygon.moc banner.moc\
		blankscrn.moc rock.moc flame.moc lines.moc laser.moc\
		pyro.moc hop.moc forest.moc swarm.moc bat.moc slip.moc\
		lissie.moc morph3d.moc blob.moc
noinst_HEADERS = saver.h polygon.h banner.h\
		blankscrn.h kscreensave.h xlock.h rock.h flame.h lines.h\
		laser.h pyro.h hop.h forest.h swarm.h bat.h slip.h\
		lissie.h blob.h

messages:
	$(XGETTEXT) -C -ktranslate *.cpp -x ../../kdelibs/po/kde.po # && mv messages.po ../po/klock.po 

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

bat.moc: bat.h
	$(MOC) bat.h -o bat.moc

slip.moc: slip.h
	$(MOC) slip.h -o slip.moc

lissie.moc: lissie.h
	$(MOC) lissie.h -o lissie.moc

blob.moc: blob.h
	$(MOC) blob.h -o blob.moc

morph3d.moc: morph3d.h
	$(MOC) morph3d.h -o morph3d.moc

