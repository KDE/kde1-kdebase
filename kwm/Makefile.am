####### kpanel Makefile.am
## written by Stephan Kulow using the old Makefile by Matthias Ettrich
##

bin_PROGRAMS 	= kwm
INCLUDES	=  @all_includes@ 
DEFS		+= -DSHAPE 
kwm_LDADD 	= @all_libraries@ -lkfm -lkdecore -lqt -lXext -lX11 
MOC		= @MOC@
CLEANFILES	= $(BUILT_SOURCES)

#######	Files

kwm_SOURCES	= client.C manager.C main.C minicli.C drag.C logout.C kwm.C task.C
BUILT_SOURCES	= client.moc manager.moc main.moc minicli.moc logout.moc task.moc


messages:
	@XGETTEXT@ -C --keyword=translate -x ../../kdelibs/po/kde.po $(kwm_SOURCES) 

%.moc: %.h
	$(MOC) $< -o $@      
