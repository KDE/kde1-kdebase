####### kpanel Makefile.am
## written by Stephan Kulow using the old Makefile by Matthias Ettrich
##

bin_PROGRAMS 	= kwm
INCLUDES	=  $(all_includes)
DEFS		+= -DSHAPE 
kwm_LDADD 	= $(all_libraries) -lkfm -lkdecore -lqt -lXext -lX11 $(LIBSOCKET) $(KDE_RPATH)
CLEANFILES	= $(BUILT_SOURCES)
SUBDIRS 	= pics

#######	Files

kwm_SOURCES	= client.C manager.C main.C minicli.C drag.C logout.C task.C
BUILT_SOURCES	= client.moc manager.moc main.moc minicli.moc logout.moc task.moc

client.C:client.moc
client.moc: client.h
	$(MOC) client.h -o client.moc

manager.C:manager.moc
manager.moc: manager.h
	$(MOC) manager.h -o manager.moc

main.C:main.moc
main.moc: main.h
	$(MOC) main.h -o main.moc

minicli.C:minicli.moc
minicli.moc: minicli.h
	$(MOC) minicli.h -o minicli.moc

logout.C:logout.moc
logout.moc: logout.h
	$(MOC) logout.h -o logout.moc

task.C:task.moc
task.moc: task.h
	$(MOC) task.h -o task.moc

messages:
	$(XGETTEXT) -C -ktranslate -x ../../kdelibs/po/kde.pot $(kwm_SOURCES) 

