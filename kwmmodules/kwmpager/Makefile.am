####### Fiddle here

INCLUDES= $(all_includes)
LDADD	= $(all_libraries) -lkdeui -lkdecore -lqt -lXext -lX11 $(LIBSOCKET) $(KDE_RPATH)

#######	Files

appdir = $(kde_appsdir)/System/
bin_PROGRAMS	= kwmpager
kwmpager_SOURCES = desktop.cpp pager.cpp
BUILTSOURCES =	pager.moc desktop.moc
DISTCLEANFILES = $(BUILTSOURCES)
noinst_HEADERS = pager.h desktop.h version.h

EXTRA_DIST = kwmpager.kdelnk

pager.cpp: pager.moc
pager.moc: pager.h
	$(MOC) pager.h -o pager.moc

desktop.cpp: desktop.moc
desktop.moc: desktop.h
	$(MOC) desktop.h -o desktop.moc

install-data-local:
	$(mkinstalldirs) $(appdir)
	$(INSTALL_DATA) kwmpager.kdelnk $(appdir)

uninstall-local:
	-rm -f $(appdir)/kwmpager.kdelnk

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(kwmpager_SOURCES)
