## Makefile.am for kvt 
## written by Stephan Kulow using the old Makefile.in, that I wrote too :-)
##

INCLUDES= @all_includes@ 
LDADD	= @all_libraries@ -lkdecore -lkdeui -lqt -lXext -lX11 
MOC	= @MOC@

PICSDIR = $(libdir)/pics
DOCSDIR = $(prefix)/doc/HTML
UTILSDIR = $(prefix)/apps/Utilities

bin_PROGRAMS = kvt

kvt_SOURCES = \
	rxvt.c command.c screen.c utmp.c \
	xsetup.c debug.c resources.c main.C\
	grkelot.c

main.C: main.moc

main.moc: main.h
	$(MOC) main.h -o main.moc

install-data-local: 
	$(mkinstalldirs) $(PICSDIR)
	$(INSTALL_DATA) ./lib/pics/UNIX.xpm $(PICSDIR)
	$(mkinstalldirs) $(DOCSDIR)
	$(INSTALL_DATA) ./doc/HTML/kvt.html $(DOCSDIR)
	$(mkinstalldirs) $(UTILSDIR)
	$(INSTALL_DATA) ./apps/Utilities/UNIX.kdelnk $(UTILSDIR)

uninstall-local:
	rm -f $(PICSDIR)/UNIX.xpm
	rm -f $(DOCSDIR)/kvt.html
	rm -f $(UTILSDIR)/UNIX.kdelnk

