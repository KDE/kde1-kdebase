
# just set the variable
#APPSDIR = $(kde_appsdir)/Settings/Sound
# set the include path for X, qt and KDE
INCLUDES= $(all_includes)

bin_PROGRAMS = 	kcmsyssound

kcmsyssound_SOURCES	= main.cpp syssound.cpp

kcmsyssound_LDFLAGS = $(all_libraries) $(KDE_RPATH)

# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.
kcmsyssound_LDADD   =   -lmediatool     -lstdc++ $(LIB_KDEUI) $(LIBCOMPAT)

# this option you can leave out. Just, if you use "make dist", you need it
noinst_HEADERS = syssound.h

# just to make sure, automake makes them 
BUILTSOURCES =	syssound.moc

# if you "make distclean", this files get removed. If you want to remove
# them while "make clean", use CLEANFILES
DISTCLEANFILES = $(BUILTSOURCES)

EXTRA_DIST = kcmsyssound.xpm mini-kcmsyssound.xpm

# make messages.po. Move this one to ../po/ and "make merge" in po
messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(kcmsyssound_SOURCES) && mv messages.po ../../po/kcmsyssound.pot

# just install datas here. Use install-exec-data for scripts and etc.
# the binary itself is already installed from automake
# use mkinstalldirs, not "install -d"
# don't install a list of file. Just one file per install.
# if you have more of them, create a subdirectory with an extra Makefile 
install-data-local: 
	$(mkinstalldirs) $(kde_icondir)
	$(INSTALL_DATA) kcmsyssound.xpm $(kde_icondir)
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) mini-kcmsyssound.xpm $(kde_minidir)/kcmsyssound.xpm

# remove ALL you have installed in install-data-local or install-exec-local
uninstall-local:
	-rm -f $(kde_icondir)/kcmsyssound.xpm
	-rm -f $(kde_minidir)/kcmsyssound.xpm

# add a dependency for every moc file to be full portable
# I've added a key binding to emacs for this. 
syssound.cpp:syssound.moc
syssound.moc: syssound.h
	$(MOC) syssound.h -o syssound.moc

