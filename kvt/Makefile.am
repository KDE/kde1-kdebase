## Makefile.am for kvt 
## written by Stephan Kulow using the old Makefile.in, that I wrote too :-)
##

INCLUDES= @all_includes@ 
LDADD	= @all_libraries@ -lkdecore -lkdeui -lqt -lXext -lX11
MOC	= @MOC@
DISTCLEANFILES = main.moc

bin_PROGRAMS = kvt

data_DATA = kvt.xpm
datadir = $(kde_icondir)

kvt_SOURCES = \
	command.c screen.c utmp.c \
	xsetup.c debug.c resources.c main.C\
	grkelot.c

main.C: main.moc

main.moc: main.h
	$(MOC) main.h -o main.moc

