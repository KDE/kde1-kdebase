## Makefile.am of kdebase/kfm

INCLUDES=	@all_includes@  
LDADD	=	@all_libraries@ -lkhtmlw -lkdeui -lkdecore -lqt -lXext -lX11 @LIBSOCKET@ @LIBJPEG@ -lgif -ljscript
MOC	=	@MOC@

prefix = @prefix@
KDEINC = $(prefix)/include
BINDIR = $(prefix)/bin
LIBDIR = $(prefix)/lib

####### Files

bin_PROGRAMS = kfm 

kfm_SOURCES =	\
	kfmgui.cpp kfmview.cpp kbind.cpp main.cpp \
	kfmdlg.cpp bookmark.cpp kfmprops.cpp kfmserver_ipc.cpp \
	kfmserver_ipc2.cpp kfmserver.cpp kioserver.cpp kioserver_ipc.cpp \
	kioserver_ipc2.cpp ipc.cpp root.cpp kfmman.cpp kiojob.cpp \
	kmsgwin.cpp htmlcache.cpp autostart.cpp kfmtree.cpp xview.cpp \
	krenamewin.cpp passworddialog.cpp fileentry.cpp debug.cpp \
	kfmpaths.cpp kfmjob.cpp kfmexec.cpp kmimemagic.cpp

noinst_HEADERS =  \
	kfmgui.h kfmview.h kbind.h root.h kfmdlg.h bookmark.h \
	kfmprops.h kfmserver_ipc.h kioserver.h kioserver_ipc.h ipc.h \
	kfmserver.h kfmman.h kiojob.h kmsgwin.h htmlcache.h kfmtree.h \
	xview.h krenamewin.h passworddialog.h fileentry.h \
	config-kfm.h kfmpaths.h kfmjob.h kfmexec.h kmimemagic.h

BUILTSOURCES =	\
	kfmgui.moc kfmview.moc root.moc kfmdlg.moc bookmark.moc \
	kfmprops.moc kfmserver_ipc.moc kfmserver.moc kioserver.moc \
	kioserver_ipc.moc kfmman.moc kiojob.moc kmsgwin.moc htmlcache.moc \
	kfmtree.moc krenamewin.moc passworddialog.moc fileentry.moc \
	kfmjob.moc kfmexec.moc

CLEANFILES = $(BUILTSOURCES)

kfmgui.cpp: kfmgui.moc
kioserver.cpp: kioserver.moc
kfmdlg.cpp: kfmdlg.moc
fileentry.cpp: fileentry.moc
kfmserver.cpp: kfmserver.moc
kiojob.cpp: kiojob.moc
root.cpp: root.moc
kfmview.cpp: kfmview.moc
kfmtree.cpp: kfmtree.moc
kmsgwin.cpp: kmsgwin.moc
krenamewin.cpp: krenamewin.moc
kfmserver_ipc.cpp: kfmserver_ipc.moc
kioserver_ipc.cpp: kioserver_ipc.moc
passworddialog.cpp: passworddialog.moc
kfmprops.cpp: kfmprops.moc
kfmman.cpp: kfmman.moc
htmlcache.cpp: htmlcache.moc
bookmark.cpp: bookmark.moc
kfmjob.cpp: kfmjob.moc
kfmexec.cpp: kfmexec.moc

%.moc: %.h
	$(MOC) $< -o $@      

SUBDIRS = kfmwarn client kioslave 

install-local-exec: 
	$(INSTALL_DATA) ./unpack $(BINDIR)

install-data-local:
	(cd pics && $(MAKE) install)
	$(mkinstalldirs) $(LIBDIR)/kfm/config
	$(INSTALL_DATA) config/kfmrc $(LIBDIR)/kfm/config
	$(INSTALL_DATA) config/desktop $(LIBDIR)/kfm/config
	$(INSTALL_DATA) magic $(KDEDIR)/share
	(cd Desktop && $(MAKE) install)

IPC:    clientIPC kioIPC

clientIPC: ./ipcc/kfm.ipc
	cd ipcc ; ./ipcc.pl kfm.ipc kfmserver_ipc kfmclient_ipc ; cp kfmserver_ipc* ../ ; cp kfmclient_ipc* ../client ; cd ..

kioIPC: ./ipcc/kio.ipc
	cd ipcc ; ./ipcc.pl kio.ipc kioserver_ipc kioslave_ipc ; cp kioserver_ipc* ../ ; cp kioslave_ipc* ../kioslave ; cd ..

messages:
	@XGETTEXT@ -C --keyword=translate -x ../../kdelibs/po/de_kde.po $(kfm_SOURCES)


