####### Fiddle here

INCLUDES = @all_includes@ 
LDADD   = @all_libraries@ -lkdecore -lkdeui -lkfm -lqt -lXext -lX11 @LIBSOCKET@
MOC	=	@MOC@

#######	Files

bin_PROGRAMS	= krootwm
krootwm_SOURCES	= krootwm.C 
BUILTSOURCES =	krootwm.moc 
DISTCLEANFILES = $(BUILTSOURCES)

krootwm.C:krootwm.moc
krootwm.moc: krootwm.h
	$(MOC) krootwm.h -o krootwm.moc



