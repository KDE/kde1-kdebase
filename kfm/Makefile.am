## Makefile.am of kdebase/kfm

INCLUDES=	$(all_includes)
LDADD	=	$(all_libraries) -lkhtmlw -lkdeui -lkdecore -lqt -lXext -lX11 $(LIBSOCKET) -ljpeg -lgif -ljscript -lm $(KDE_RPATH)
CONFIGDIR = $(kde_confdir)
MIMELNKDIR = $(kde_mimedir)

data_DATA = kfm.xpm
datadir = $(kde_icondir)

####### Files

bin_PROGRAMS = kfm

kfm_SOURCES =	\
	kfmgui.cpp kfmview.cpp kbind.cpp main.cpp \
	kfmdlg.cpp bookmark.cpp kfmprops.cpp kfmserver_ipc.cpp \
	kfmserver_ipc2.cpp kfmserver.cpp kioserver.cpp kioserver_ipc.cpp \
	kioserver_ipc2.cpp kfmipc.cpp root.cpp kfmman.cpp kiojob.cpp \
	kmsgwin.cpp htmlcache.cpp autostart.cpp kfmtree.cpp xview.cpp \
	krenamewin.cpp passworddialog.cpp kURLcompletion.cpp debug.cpp kfm.cpp \
	kfmpaths.cpp kfmjob.cpp kfmexec.cpp kmimemagic.cpp kfinder.cpp utils.cpp \
	useragentdlg.cpp

noinst_HEADERS =  \
	kfmgui.h kfmview.h kbind.h root.h kfmdlg.h bookmark.h \
	kfmprops.h kfmserver_ipc.h kioserver.h kioserver_ipc.h kfmipc.h \
	kfmserver.h kfmman.h kiojob.h kmsgwin.h htmlcache.h kfmtree.h \
	xview.h krenamewin.h passworddialog.h kURLcompletion.h utils.h kfm.h \
	config-kfm.h kfmpaths.h kfmjob.h kfmexec.h kmimemagic.h kfinder.h	\
	useragentdlg.h

BUILT_SOURCES =	\
	kfmgui.moc kfmview.moc root.moc kfmdlg.moc bookmark.moc \
	kfmprops.moc kfmserver_ipc.moc kfmserver.moc kioserver.moc \
	kioserver_ipc.moc kfmman.moc kiojob.moc kmsgwin.moc htmlcache.moc \
	kfmtree.moc krenamewin.moc passworddialog.moc kURLcompletion.moc \
	kfmjob.moc kfmexec.moc cgi.moc kbind.moc kfinder.moc kfm.moc	\
	useragentdlg.moc

CLEANFILES = $(BUILT_SOURCES)


kfmgui.cpp:kfmgui.moc
kfmgui.moc: kfmgui.h
	$(MOC) kfmgui.h -o kfmgui.moc

kioserver.cpp:kioserver.moc
kioserver.moc: kioserver.h
	$(MOC) kioserver.h -o kioserver.moc

kfmdlg.cpp:kfmdlg.moc
kfmdlg.moc: kfmdlg.h
	$(MOC) kfmdlg.h -o kfmdlg.moc

kURLcompletion.cpp:kURLcompletion.moc
kURLcompletion.moc: kURLcompletion.h
	$(MOC) kURLcompletion.h -o kURLcompletion.moc

kfmserver.cpp:kfmserver.moc
kfmserver.moc: kfmserver.h
	$(MOC) kfmserver.h -o kfmserver.moc

kiojob.cpp:kiojob.moc
kiojob.moc: kiojob.h
	$(MOC) kiojob.h -o kiojob.moc

root.cpp:root.moc
root.moc: root.h
	$(MOC) root.h -o root.moc

cgi.cpp:cgi.moc
cgi.moc: cgi.h
	$(MOC) cgi.h -o cgi.moc

kfmview.cpp:kfmview.moc
kfmview.moc: kfmview.h
	$(MOC) kfmview.h -o kfmview.moc

kfmtree.cpp:kfmtree.moc
kfmtree.moc: kfmtree.h
	$(MOC) kfmtree.h -o kfmtree.moc

kmsgwin.cpp:kmsgwin.moc
kmsgwin.moc: kmsgwin.h
	$(MOC) kmsgwin.h -o kmsgwin.moc

krenamewin.cpp:krenamewin.moc
krenamewin.moc: krenamewin.h
	$(MOC) krenamewin.h -o krenamewin.moc

kfmserver_ipc.cpp:kfmserver_ipc.moc
kfmserver_ipc.moc: kfmserver_ipc.h
	$(MOC) kfmserver_ipc.h -o kfmserver_ipc.moc

kioserver_ipc.cpp:kioserver_ipc.moc
kioserver_ipc.moc: kioserver_ipc.h
	$(MOC) kioserver_ipc.h -o kioserver_ipc.moc

passworddialog.cpp:passworddialog.moc
passworddialog.moc: passworddialog.h
	$(MOC) passworddialog.h -o passworddialog.moc

kfmprops.cpp:kfmprops.moc
kfmprops.moc: kfmprops.h
	$(MOC) kfmprops.h -o kfmprops.moc

kfmman.cpp:kfmman.moc
kfmman.moc: kfmman.h
	$(MOC) kfmman.h -o kfmman.moc

htmlcache.cpp:htmlcache.moc
htmlcache.moc: htmlcache.h
	$(MOC) htmlcache.h -o htmlcache.moc

bookmark.cpp:bookmark.moc
bookmark.moc: bookmark.h
	$(MOC) bookmark.h -o bookmark.moc

kfmjob.cpp:kfmjob.moc
kfmjob.moc: kfmjob.h
	$(MOC) kfmjob.h -o kfmjob.moc

kfmexec.cpp:kfmexec.moc
kfmexec.moc: kfmexec.h
	$(MOC) kfmexec.h -o kfmexec.moc

kbind.cpp:kbind.moc
kbind.moc: kbind.h
	$(MOC) kbind.h -o kbind.moc

kfinder.cpp:kfinder.moc
kfinder.moc: kfinder.h
	$(MOC) kfinder.h -o kfinder.moc

kfm.cpp:kfm.moc
kfm.moc: kfm.h
	$(MOC) kfm.h -o kfm.moc
     
useragentdlg.cpp:useragentdlg.moc
useragentdlg.moc: useragentdlg.h
	$(MOC) useragentdlg.h -o useragentdlg.moc
     
SUBDIRS = kfmwarn client kioslave pics Desktop kfmexec

install-local-exec: 
	$(mkinstalldirs) $(bindir)
	$(INSTALL_DATA) ./unpack $(bindir)
	$(INSTALL_SCRIPT) ./kfmsu $(bindir)
	$(INSTALL_SCRIPT) ./kfmsu2 $(bindir)
	$(INSTALL_SCRIPT) ./startkde $(bindir)

install-data-local:
	$(mkinstalldirs) $(CONFIGDIR)
	$(INSTALL_DATA) config/kfmrc $(CONFIGDIR)
	$(INSTALL_DATA) config/desktop $(CONFIGDIR)
	$(mkinstalldirs) $(MIMELNKDIR)
	$(INSTALL_DATA) magic $(MIMELNKDIR)
	$(INSTALL_SCRIPT) ./kfmsu $(bindir)
	$(INSTALL_SCRIPT) ./kfmsu2 $(bindir)
	$(INSTALL_SCRIPT) ./startkde $(bindir)

uninstall-local:
	-rm -f $(bindir)/unpack
	-rm -f $(bindir)/kfmsu
	-rm -f $(bindir)/kfmsu2
	-rm -f $(bindir)/startkde
	-rm -f $(CONFIGDIR)/kfmrc
	-rm -f $(CONFIGDIR)/desktop
	-rm -f $(MIMELNKDIR)/magic

IPC:    clientIPC kioIPC

clientIPC: ./ipcc/kfm.ipc
	cd ipcc ; ./ipcc.pl kfm.ipc kfmserver_ipc kfmclient_ipc ; cp kfmserver_ipc* ../ ; cp kfmclient_ipc* ../client ; cd ..

kioIPC: ./ipcc/kio.ipc
	cd ipcc ; ./ipcc.pl kio.ipc kioserver_ipc kioslave_ipc ; cp kioserver_ipc* ../ ; cp kioslave_ipc* ../kioslave ; cd ..

messages:
	$(XGETTEXT) -C -ktranslate -kaliasLocale -x ../../kdelibs/po/kde.pot $(kfm_SOURCES) kfmexec/main.cpp && mv messages.po ../po/kfm.pot

