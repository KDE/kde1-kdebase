# this 10 paths are KDE specific. Use them:
# kde_htmldir       Where your docs should go to. (contains lang subdirs)
# kde_appsdir       Where your application file (.kdelnk) should go to. 
# kde_icondir       Where your icon should go to.
# kde_minidir       Where your mini icon should go to.
# kde_datadir       Where you install application data. (Use a subdir)
# kde_locale        Where translation files should go to.(contains lang subdirs)
# kde_cgidir        Where cgi-bin executables should go to.
# kde_confdir       Where config files should go to.
# kde_mimedir       Where mimetypes should go to.
# kde_toolbardir    Where general toolbar icons should go to.
# kde_wallpaperdir  Where general wallpapers should go to.

# just set the variable
INCLUDES= $(all_includes)
SUBDIRS = doc maps

####### This part is very kless specific
# you can add here more. This one gets installed 
bin_PROGRAMS = 	kikbd kcmikbd
BUILT_SOURCES = kconfobjs.moc.h  kobjconf.moc.h \
	widgets.moc.h keymap.moc.h kikbd.moc.h kikbdconf.moc.h

kikbd_SOURCES = kikbd.cpp keytrans.cpp keymap.cpp kikbdconf.cpp \
	kobjconf.cpp kconfobjs.cpp
kikbd_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kikbd_LDADD   = $(LIB_KDEUI)

kcmikbd_SOURCES = widgets.cpp kcmikbd.cpp kikbdconf.cpp \
	kobjconf.cpp kconfobjs.cpp
kcmikbd_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kcmikbd_LDADD   = $(LIB_KDEUI)

# this option you can leave out. Just, if you use "make dist", you need it
noinst_HEADERS = 

# if you "make distclean", this files get removed. If you want to remove
# them while "make clean", use CLEANFILES
CLEANFILES = $(BUILT_SOURCES)

# make messages.po. Move this one to ../po/ and "make merge" in po
messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(kikbd_SOURCES) $(kcmikbd_SOURCES) && mv messages.po ../po/kikbd.pot

kdelnk_DATA = kikbd.kdelnk 
kdelnkdir = $(kde_appsdir)/System

kdelnk2_DATA = kcmikbd.kdelnk
kdelnk2dir = $(kde_appsdir)/Settings/Input_Devices

icon_DATA = kikbd.xpm
icondir = $(kde_icondir)

config_DATA = kikbdrc
configdir = $(kde_datadir)/kikbd

install-data-local: kcmikbd.kdelnk
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) mini-kikbd.xpm $(kde_minidir)/kikbd.xpm

uninstall-local:
	-rm -f $(kde_minidir)/kikbd.xpm

kconfobjs.moc.h: kconfobjs.h
	$(MOC) -o kconfobjs.moc.h kconfobjs.h
kobjconf.moc.h: kobjconf.h
	$(MOC) -o kobjconf.moc.h kobjconf.h
widgets.moc.h: widgets.h
	$(MOC) -o widgets.moc.h widgets.h
keymap.moc.h: keymap.h
	$(MOC) -o keymap.moc.h keymap.h
kikbd.moc.h: kikbd.h
	$(MOC) -o kikbd.moc.h kikbd.h
kikbdconf.moc.h: kikbdconf.h
	$(MOC) -o kikbdconf.moc.h kikbdconf.h
kconfobjs.cpp:kconfobjs.moc.h
keymap.cpp:keymap.moc.h
kikbd.cpp:kikbd.moc.h
kikbdconf.cpp:kikbdconf.moc.h
kobjconf.cpp:kobjconf.moc.h
widgets.cpp:widgets.moc.h   

