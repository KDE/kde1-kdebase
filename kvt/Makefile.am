## Makefile.am for kvt 
## written by Stephan Kulow using the old Makefile.in, that I wrote too :-)
##

INCLUDES= $(all_includes)
LDADD	= $(all_libraries) -lkdecore -lkdeui -lqt -lXext -lX11 $(KDE_RPATH)
DISTCLEANFILES = main.moc

bin_PROGRAMS = kvt

kvt_SOURCES = \
	command.c screen.c utmp.c \
	xsetup.c debug.c resources.c main.C\
	grkelot.c

main.C: main.moc

main.moc: main.h
	$(MOC) main.h -o main.moc
	
install-data-local: 
	$(mkinstalldirs) $(kde_icondir)
	$(INSTALL_DATA) kvt.xpm $(kde_icondir)/kvt.xpm
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) mini-kvt.xpm $(kde_minidir)/kvt.xpm

uninstall-local:
	rm -f $(kde_icondir)/kvt.xpm
	rm -f $(kde_minidir)/kvt.xpm

