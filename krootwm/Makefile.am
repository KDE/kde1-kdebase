####### Fiddle here

INCLUDES = @all_includes@ 
LDADD   = @all_libraries@ -lkdecore -lkfm -lqt -lXext -lX11 
MOC	=	@MOC@

#######	Files

bin_PROGRAMS	= krootwm
krootwm_SOURCES	= krootwm.C kwmmapp.C kwm.C
BUILTSOURCES =	krootwm.moc

%.moc: %.h
	$(MOC) $< -o $@      


