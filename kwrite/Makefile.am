INCLUDES = $(all_includes)
LDFLAGS = -lkfile -lkfm -lkdeui -lkdecore -lqt -lXext -lX11 $(all_libraries) $(KDE_RPATH)

bin_PROGRAMS = kwrite

kwrite_SOURCES = highlight.cpp kwdoc.cpp kwview.cpp \
	hldialog.cpp kwdialog.cpp toplevel.cpp listedit.cpp editlinedlg.cpp

noinst_HEADERS = highlight.h kwdialog.h kwview.h toplevel.h hldialog.h \
	kwdoc.h	listedit.h 

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
