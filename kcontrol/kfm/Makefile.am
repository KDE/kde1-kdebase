
INCLUDES = $(all_includes) 
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD =       $(LIB_KDEUI) 

bin_PROGRAMS = kcmkfm

kcmkfm_SOURCES = main.cpp htmlopts.cpp miscopts.cpp
BUILT_SOURCES = htmlopts.moc miscopts.moc
noinst_HEADERS = htmlopts.h miscopts.h

messages:
	$(XGETTEXT) -C -ki18n -x $(includedir)/kde.pot $(kcmkfm_SOURCES) && mv messages.po ../../po/kcmkfm.pot

DISTCLEANFILES = $(BUILT_SOURCES)

htmlopts.cpp:htmlopts.moc
htmlopts.moc: htmlopts.h
	$(MOC) htmlopts.h -o htmlopts.moc

miscopts.cpp:miscopts.moc
miscopts.moc: miscopts.h
	$(MOC) miscopts.h -o miscopts.moc

