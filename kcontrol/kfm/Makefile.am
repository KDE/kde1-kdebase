INCLUDES = $(all_includes) 
LDADD = $(all_libraries) -lkdeui -lkdecore -lqt -lXext -lX11 $(LIBSOCKET) $(KDE_RPATH)

bin_PROGRAMS = kcmkfm

kcmkfm_SOURCES = main.cpp htmlopts.cpp
BUILT_SOURCES = htmlopts.moc
noinst_HEADERS = htmlopts.h

messages:
	$(XGETTEXT) -C -ki18n -x ../../../kdelibs/po/kde.pot $(kcmkfm_SOURCES) && mv messages.po ../../po/kcmkfm.pot

DISTCLEANFILES = $(BUILT_SOURCES)

htmlopts.cpp:htmlopts.moc
htmlopts.moc: htmlopts.h
	$(MOC) htmlopts.h -o htmlopts.moc

