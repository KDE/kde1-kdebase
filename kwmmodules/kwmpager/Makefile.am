####### Fiddle here

INCLUDES= @all_includes@ 
LDADD	= @all_libraries@ -lkdeui -lkdecore -lqt -lXext -lX11 @LIBSOCKET@
CXXFLAGS += -export-dynamic

#######	Files

appdir = $(kde_appsdir)/System/
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


libdir = $(kcc_moduledir)
# lib_LTLIBRARIES = libpager.la

# libpager_la_SOURCES = module.cpp pagercfg.cpp
# libpager_la_LDFLAGS = -version-info 0:1

pagercfg.cpp:pagercfg.moc
pagercfg.moc: pagercfg.h
	$(MOC) pagercfg.h -o pagercfg.moc

