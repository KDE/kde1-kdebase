INCLUDES = $(all_includes)
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD =       $(LIB_KFILE)

bin_PROGRAMS = kwrite

kwrite_SOURCES = highlight.cpp kwdoc.cpp kwview.cpp \
	kwdialog.cpp toplevel.cpp kmimemagic.cpp

noinst_HEADERS = highlight.h kwdialog.h kwview.h toplevel.h \
	kwdoc.h	kmimemagic.h

kwrite_METASOURCES = USE_AUTOMOC

SUBDIRS = docs

lnkdir = $(kde_appsdir)/Applications
lnk_DATA = kwrite.kdelnk

icondir = $(kde_icondir)
icon_DATA = kwrite.xpm undo.xpm redo.xpm

EXTRA_DIST = $(lnk_DATA) $(icon_DATA) mini-kwrite.xpm

install-data-local: mini-kwrite.xpm
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) $(srcdir)/mini-kwrite.xpm $(kde_minidir)/kwrite.xpm
	$(mkinstalldirs) $(kde_minidir)/../large
	$(INSTALL_DATA) $(srcdir)/large-kwrite.xpm $(kde_minidir)/../large/kwrite.xpm

uninstall-local:
	-rm -f $(kde_minidir)/kwrite.xpm
	-rm -f $(kde_minidir)/../large/kwrite.xpm

messages:
	$(XGETTEXT) -C -ki18n -ktranslate -x $(includedir)/kde.pot $(kwrite_SOURCES) && mv messages.po ../po/kwrite.pot

