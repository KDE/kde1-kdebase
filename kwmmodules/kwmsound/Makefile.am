
INCLUDES = $(all_includes)

LDFLAGS	= $(all_libraries) $(KDE_RPATH)
LDADD =   -lmediatool     -lstdc++ $(LIB_KDEUI) $(LIBCOMPAT)

bin_PROGRAMS = kwmsound
kwmsound_SOURCES = kwmsound.C
noinst_HEADERS = kwmsound.h

BUILTSOURCES = kwmsound.moc
DISTCLEANFILES = $(BUILTSOURCES)

kwmsound.C:kwmsound.moc
kwmsound.moc: kwmsound.h
	$(MOC) kwmsound.h -o kwmsound.moc

