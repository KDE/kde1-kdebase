INCLUDES = $(all_includes)
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD = -lkfile -lkfm     $(LIB_KDEUI)

bin_PROGRAMS = kwrite

kwrite_SOURCES = highlight.cpp kwdoc.cpp kwview.cpp \
	kwdialog.cpp toplevel.cpp kmimemagic.cpp

noinst_HEADERS = highlight.h kwdialog.h kwview.h toplevel.h \
	kwdoc.h	kmimemagic.h

kwrite_METASOURCES = USE_AUTOMOC


lnkdir = $(kde_appsdir)/Applications
lnk_DATA = kwrite.kdelnk

icondir = $(kde_icondir)
icon_DATA = kwrite.xpm

EXTRA_DIST = $(lnk_DATA) $(icon_DATA) mini-kwrite.xpm

install-data-local: mini-kwrite.xpm
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) mini-kwrite.xpm $(kde_minidir)/kwrite.xpm

uninstall-local:
	-rm -f $(kde_minidir)/kwrite.xpm

messages:
	$(XGETTEXT) -C -ki18n -ktranslate -x $(includedir)/kde.pot $(kwrite_SOURCES) && mv messages.po ../po/kwrite.pot

