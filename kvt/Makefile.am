## Makefile.am for kvt 
## written by Stephan Kulow using the old Makefile.in, that I wrote too :-)
##

INCLUDES= @all_includes@ 
LDADD	= @all_libraries@ -lkdecore -lqt -lXext -lX11 
MOC	= @MOC@

PICSDIR = $(libdir)/pics
DOCSDIR = $(prefix)/doc/HTML
UTILSDIR = $(prefix)/apps/Utilities

#ifdef  GREEK_SUPPORT
GREEK_EXTRA=-DGREEK_KBD
GREEK_SRC=grkelot.c
GREEK_OBJ=grkelot.o
#else
GREEK_EXTRA=    #-DGREEK_KBD
GREEK_SRC=      #grkelot.c
GREEK_OBJ=      #grkelot.o
#endif

CFLAGS += -DBSD -DTTYTAB=\"/etc/ttytab\" -DPRINT_PIPE $(GREEK_EXTRA) -DMAX_REFRESH_PERIOD=10 -DFAKE_RESOURCES -DUSE_XCOPYAREA -DCOLOR 

bin_PROGRAMS = kvt

kvt_SOURCES = \
	rxvt.c command.c screen.c utmp.c \
	xsetup.c debug.c resources.c main.C\
	$(GREEK_SRC)

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

