## Makefile.am for kvt 
## written by Stephan Kulow using the old Makefile.in, that I wrote too :-)
##

INCLUDES= $(all_includes)
LDADD	=       $(LIBCURSES) $(LIB_KDEUI)
LDFLAGS = $(all_libraries) $(KDE_RPATH)
DISTCLEANFILES = main.moc

bin_PROGRAMS = kvt

kvt_SOURCES = \
	command.c screen.c utmp.c \
	xsetup.c debug.c resources.c main.C\
	grkelot.c

noinst_HEADERS = debug.h rxvt.h command.h screen.h xsetup.h sbar.h kvt_version.h main.h

EXTRA_DIST = kvt.xpm mini-kvt.xpm

main.C: main.moc

main.moc: main.h
	$(MOC) $(srcdir)/main.h -o main.moc

icon_DATA = kvt.xpm
icondir = $(kde_icondir)

install-data-local: 
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) $(srcdir)/mini-kvt.xpm $(kde_minidir)/kvt.xpm

uninstall-local:
	rm -f $(kde_minidir)/kvt.xpm

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(kvt_SOURCES) && mv messages.po ../po/kvt.pot
