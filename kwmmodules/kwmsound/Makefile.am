
INCLUDES = $(all_includes)

LDFLAGS	= $(all_libraries) $(KDE_RPATH)
LDADD = -lkdeui -lkdecore -lqt -lXext -lX11 -lmediatool $(LIBSOCKET) 

bin_PROGRAMS = kwmsound
kwmsound_SOURCES = kwmsound.C

BUILTSOURCES = kwmsound.moc
DISTCLEANFILES = $(BUILTSOURCES)

kwmsound.C:kwmsound.moc
kwmsound.moc: kwmsound.h
	$(MOC) kwmsound.h -o kwmsound.moc

