
INCLUDES = $(all_includes) 
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD = -lkdeui -lkdecore -lqt -lXext -lX11 $(LIBSOCKET) 

bin_PROGRAMS = kcmkfm

kcmkfm_SOURCES = main.cpp htmlopts.cpp
BUILT_SOURCES = htmlopts.moc
noinst_HEADERS = htmlopts.h

messages:
	$(XGETTEXT) -C -ki18n -x $(includedir)/kde.pot $(kcmkfm_SOURCES) && mv messages.po ../../po/kcmkfm.pot

DISTCLEANFILES = $(BUILT_SOURCES)

htmlopts.cpp:htmlopts.moc
htmlopts.moc: htmlopts.h
	$(MOC) htmlopts.h -o htmlopts.moc

