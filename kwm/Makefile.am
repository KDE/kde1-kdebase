####### kpanel Makefile.am
## written by Stephan Kulow using the old Makefile by Matthias Ettrich
##

bin_PROGRAMS 	= kwm
INCLUDES	=  $(all_includes)
LDFLAGS 	= $(all_libraries) $(KDE_RPATH)
kwm_LDADD 	= $(LIB_KFM) $(LIB_KDEUI)
SUBDIRS 	= pics

#######	Files

kwm_SOURCES	= client.C manager.C main.C minicli.C warning.C drag.C \
	logout.C taskmgr.C kURLcompletion.cpp gradientFill.C 
METASOURCES	= client.moc manager.moc main.moc minicli.moc \
	warning.moc logout.moc taskmgr.moc kURLcompletion.moc 
noinst_HEADERS = client.h options.h manager.h main.h gradientFill.h \
	minicli.h logout.h taskmgr.h warning.h version.h kURLcompletion.h 

EXTRA_DIST = kwm.xpm mini-kwm.xpm CHANGES LICENSE.readme images

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(kwm_SOURCES) && mv messages.po ../po/kwm.pot

kde_icon_DATA = kwm.xpm

install-data-local: 
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) $(srcdir)/mini-kwm.xpm $(kde_minidir)/kwm.xpm
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) $(srcdir)/large-kwm.xpm $(kde_minidir)/../large/kwm.xpm

uninstall-local:
	rm -f $(kde_minidir)/kwm.xpm

main.C: kwmbindings.cpp

