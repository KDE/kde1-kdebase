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
APPSDIR = $(kde_appsdir)/System
kikbd_conf = $(kde_datadir)/kikbd
# set the include path for X, qt and KDE
INCLUDES= $(all_includes)
# claim, which subdirectories you want to install
SUBDIRS = kcmikbd doc

####### This part is very kless specific
# you can add here more. This one gets installed 
bin_PROGRAMS = 	kikbd

# Which sources should be compiled for kless.
kikbd_SOURCES = kikbd.cpp keytrans.cpp \
    keymap.cpp kikbdconf.cpp kobjconf.cpp

kikbd_METASOURCES = USE_AUTOMOC

# the library search path. 
kikbd_LDFLAGS = $(all_libraries) $(KDE_RPATH)

# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.
kikbd_LDADD   = -lkdeui -lkdecore -lqt -lXext -lX11 $(LIBSOCKET)

# this option you can leave out. Just, if you use "make dist", you need it
noinst_HEADERS = 

# if you "make distclean", this files get removed. If you want to remove
# them while "make clean", use CLEANFILES
DISTCLEANFILES = $(kikbd_METASOURCES)

# make messages.po. Move this one to ../po/ and "make merge" in po
messages:
	$(XGETTEXT) -C -ktranslate $(kikbd_SOURCES)

# just install datas here. Use install-exec-local for scripts and etc.
# the binary itself is already installed from automake
# use mkinstalldirs, not "install -d"
# don't install a list of file. Just one file per install.
# if you have more of them, create a subdirectory with an extra Makefile 
install-data-local: 
	$(mkinstalldirs) $(kikbd_conf)
	$(INSTALL_DATA) maps/*.kimap $(kikbd_conf)
	$(INSTALL_DATA) maps/mkmap $(kikbd_conf)
	$(INSTALL_DATA) kikbd.kdelnk $(kde_appsdir)/System/kikbd.kdelnk
	$(INSTALL_DATA) mini-kikbd.xpm $(kde_minidir)/kikbd.xpm
	$(INSTALL_DATA) kikbd.xpm $(kde_icondir)/kikbd.xpm
	$(INSTALL_DATA) kikbdrc $(kde_confdir) 

# remove ALL you have installed in install-data-local or install-exec-local
uninstall-local:
	-rm -f $(APPSDIR)/kikbd.kdelnk
	-rm -f $(kde_icondir)/kikbd.xpm
	-rm -f $(kde_minidir)/kikbd.xpm






