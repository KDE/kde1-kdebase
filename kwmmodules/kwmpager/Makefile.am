####### Fiddle here

INCLUDES = @all_includes@ 
LDADD   = @all_libraries@ -lkdeui -lkdecore -lqt -lXext -lX11 @LIBSOCKET@
MOC	=	@MOC@

#######	Files

appdir = $(prefix)/share/applnk/System/

bin_PROGRAMS	= kwmpager
kwmpager_SOURCES = desktop.cpp pager.cpp 
BUILTSOURCES =	pager.moc desktop.moc
DISTCLEANFILES = $(BUILTSOURCES)

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

