####### Fiddle here

INCLUDES = @all_includes@ 
LDADD   = @all_libraries@ -lkfm -lkdeui -lkdecore -lqt -lXext -lX11 @LIBSOCKET@

#######	Files

bin_PROGRAMS	= krootwm
krootwm_SOURCES	= krootwm.C 
BUILTSOURCES =	krootwm.moc 
DISTCLEANFILES = $(BUILTSOURCES)

krootwm.C:krootwm.moc
krootwm.moc: krootwm.h
	$(MOC) krootwm.h -o krootwm.moc



