## Makefile.am of kdebase/kfm

INCLUDES= $(all_includes)
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD	= $(LIB_KHTMLW) $(LIB_KFILE)
CONFIGDIR = $(kde_confdir)
MIMELNKDIR = $(kde_mimedir)

icon_DATA = kfm.xpm link.xpm
icondir = $(kde_icondir)

data_DATA = directory.trash directory.autostart directory.templates directory.desktop
datadir = $(kde_datadir)/kfm

EXTRA_DIST = kfm.xpm link.xpm directory.trash directory.autostart directory.templates directory.desktop kfmsu kfmsu2 startkde kde unpack magic ipcc

####### Files

bin_SCRIPTS = kfmsu kfmsu2 startkde kde unpack

bin_PROGRAMS = kfm

kfm_SOURCES =	\
	kfmgui.cpp kfmview.cpp kbind.cpp main.cpp \
	kfmdlg.cpp bookmark.cpp kfmprops.cpp kfmserver_ipc.cpp \
	kfmserver_ipc2.cpp kfmserver.cpp kioserver.cpp kioserver_ipc.cpp \
	kioserver_ipc2.cpp kfmipc.cpp root.cpp kfmman.cpp kiojob.cpp \
	htmlcache.cpp autostart.cpp kfmtree.cpp \
	krenamewin.cpp passworddialog.cpp kURLcompletion.cpp debug.cpp kfmw.cpp \
	kfmpaths.cpp kfmjob.cpp kfmexec.cpp kmimemagic.cpp kfinder.cpp utils.cpp \
	open-with.cpp finddlg.cpp kcookiejar.cpp kcookiewin.cpp popup.cpp kintlist.cpp

noinst_HEADERS =  \
	kfmgui.h kfmview.h kbind.h root.h kfmdlg.h bookmark.h \
	kfmprops.h kfmserver_ipc.h kioserver.h kioserver_ipc.h kfmipc.h \
	kfmserver.h kfmman.h kiojob.h htmlcache.h kfmtree.h \
	krenamewin.h passworddialog.h kURLcompletion.h utils.h kfmw.h \
	config-kfm.h kfmpaths.h kfmjob.h kfmexec.h kmimemagic.h kfinder.h \
	open-with.h finddlg.h kcookiejar.h kcookiewin.h popup.h \
	kstrlist.h kintlist.h

METASOURCES =	\
	kfmgui.moc kfmview.moc kbind.moc \
	kfmdlg.moc bookmark.moc kfmprops.moc kfmserver_ipc.moc \
	kfmserver.moc kioserver.moc kioserver_ipc.moc \
	root.moc kfmman.moc kiojob.moc  \
	htmlcache.moc kfmtree.moc \
	krenamewin.moc passworddialog.moc kURLcompletion.moc kfmw.moc \
	kfmjob.moc kfmexec.moc kfinder.moc  \
	open-with.moc finddlg.moc kcookiewin.moc popup.moc

SUBDIRS = kfmwarn client kioslave pics Desktop kfmexec config

install-data-local:
	$(mkinstalldirs) $(CONFIGDIR)
	$(INSTALL_DATA) $(srcdir)/config/kfmrc $(CONFIGDIR)
	$(INSTALL_DATA) $(srcdir)/config/desktop $(CONFIGDIR)
	$(mkinstalldirs) $(MIMELNKDIR)
	$(INSTALL_DATA) $(srcdir)/magic $(MIMELNKDIR)

uninstall-local:
	-rm -f $(CONFIGDIR)/kfmrc
	-rm -f $(CONFIGDIR)/desktop
	-rm -f $(MIMELNKDIR)/magic

IPC:    clientIPC kioIPC

clientIPC: ./ipcc/kfm.ipc
	cd ipcc ; ./ipcc.pl kfm.ipc kfmserver_ipc kfmclient_ipc ; cp kfmserver_ipc* ../ ; cp kfmclient_ipc* ../client ; cd ..

kioIPC: ./ipcc/kio.ipc
	cd ipcc ; ./ipcc.pl kio.ipc kioserver_ipc kioslave_ipc ; cp kioserver_ipc* ../ ; cp kioslave_ipc* ../kioslave ; cd ..

messages:
	$(XGETTEXT) -C -ktranslate -kaliasLocale -ki18n -x $(includedir)/kde.pot $(kfm_SOURCES) $(noinst_HEADERS) kfmexec/main.cpp && mv messages.po ../po/kfm.pot

