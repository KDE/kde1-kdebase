####### kpanel Makefile.am
## written by Stephan Kulow using the old Makefile by Matthias Ettrich
##

INCLUDES	= $(all_includes)

CXXFLAGS = $(NOOPT_CXXFLAGS)

bin_PROGRAMS = kpanel
kpanel_LDADD =	$(LIB_KFM) $(LIB_KDEUI) 
LDFLAGS = $(all_libraries) $(KDE_RPATH)
kpanel_SOURCES = kpanel.C props.C layout.C tools.C main.C events.C pmenu.C pfilemenu.C
noinst_HEADERS = kpanel.h pmenu.h kpanel_version.h
METASOURCES = kpanel.moc pmenu.moc 
SUBDIRS = pics sample_kdelnk_files

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(kpanel_SOURCES) && mv messages.po ../po/kpanel.pot

defaultdir = $(kde_datadir)/kpanel/default
default_DATA = Home.kdelnk Desktop.kdelnk

install-data-local:
	$(mkinstalldirs) $(defaultdir)
	$(INSTALL_DATA) $(srcdir)/personal.kdelnk $(defaultdir)/personal_directory

uninstall-local:
	-rm -f $(defaultdir)/personal_directory

EXTRA_DIST = CHANGES Desktop.kdelnk Home.kdelnk LICENSE.readme \
	README.kdisknav-devel SWALLOW.readme 
