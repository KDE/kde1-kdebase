####### Fiddle here

INCLUDES = @all_includes@ 
LDADD   = @all_libraries@ -lkdecore -lkfm -lqt -lXext -lX11 @LIBSOCKET@
MOC	=	@MOC@

#######	Files

bin_PROGRAMS	= krootwm
krootwm_SOURCES	= krootwm.C kwmmapp.C kwm.C
BUILTSOURCES =	krootwm.moc kwmapp.moc
DISTCLEANFILES = $(BUILTSOURCES)

krootwm.C:krootwm.moc
krootwm.moc: krootwm.h
	$(MOC) krootwm.h -o krootwm.moc

kwmapp.C:kwmapp.moc
kwmapp.moc: kwmapp.h
	$(MOC) kwmapp.h -o kwmapp.moc




