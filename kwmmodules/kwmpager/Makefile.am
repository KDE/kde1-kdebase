####### Fiddle here

INCLUDES= $(all_includes)
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD	=       $(LIB_KDEUI)

#######	Files

appdir = $(kde_appsdir)/System/
bin_PROGRAMS	= kwmpager
kwmpager_SOURCES = desktop.cpp pager.cpp
METASOURCES =	pager.moc desktop.moc
noinst_HEADERS = pager.h desktop.h version.h

EXTRA_DIST = kwmpager.kdelnk

install-data-local:
	$(mkinstalldirs) $(appdir)
	$(INSTALL_DATA) $(srcdir)/kwmpager.kdelnk $(appdir)

uninstall-local:
	-rm -f $(appdir)/kwmpager.kdelnk

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(kwmpager_SOURCES)
