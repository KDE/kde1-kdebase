#
# Makefile.am for kscreensave.   Good Luck
#

MOC = @MOC@
INCLUDES = @all_includes@
LDADD = @all_libraries@ -L. -lkdeui -lkdecore -lqt -lXext -lX11 @LIBCRYPT@
CLEANFILES = $(BUILTSOURCES)

# for shadow passwords override the value
INSTALL = @INSTALL@ $(SETUIDFLAGS)

# automake doesn't accept \ within bin_PROGRAMS, don't ask me, why
bin_PROGRAMS = kpolygon.kss kswarm.kss klissie.kss kbanner.kss klines.kss kblankscrn.kss krock.kss kflame.kss klaser.kss kpyro.kss khop.kss kbat.kss kslip.kss kforest.kss klock

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

klock_SOURCES = klock.cpp kscreensave.cpp


BUILT_SOURCES = main.moc saver.moc polygon.moc banner.moc\
		blankscrn.moc rock.moc flame.moc lines.moc laser.moc\
		pyro.moc hop.moc forest.moc swarm.moc bat.moc slip.moc\
		lissie.moc
noinst_HEADERS = saver.h polygon.h banner.h\
		blankscrn.h kscreensave.h xlock.h rock.h flame.h lines.h\
		laser.h pyro.h hop.h forest.h swarm.h bat.h slip.h\
		lissie.h

########## Meta objects ##########

%.moc: %.h
	$(MOC) $< -o $@

