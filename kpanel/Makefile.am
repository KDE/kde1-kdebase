####### kpanel Makefile.am
## written by Stephan Kulow using the old Makefile by Matthias Ettrich
##

INCLUDES	= $(all_includes)
DISTCLEANFILES 	= $(BUILT_SOURCES)

CXXFLAGS = @NOOPT_CXXFLAGS@

bin_PROGRAMS = kpanel
kpanel_LDADD =	$(all_libraries) -lkfm -lkdeui -lkdecore -lqt -lXext -lX11 $(LIBSOCKET) $(KDE_RPATH)
kpanel_SOURCES = kpanel.C props.C layout.C tools.C main.C events.C pmenu.C pfilemenu.C
noinst_HEADERS = kpanel.h pmenu.h kpanel_version.h
BUILT_SOURCES = kpanel.moc pmenu.moc 
SUBDIRS = pics

kdisknavdir = $(kde_datadir)/kdisknav

main.C: kpanel.moc
pmenu.C: pmenu.moc

kpanel.moc: kpanel.h
	$(MOC) kpanel.h -o kpanel.moc
pmenu.moc: pmenu.h
	$(MOC) pmenu.h -o pmenu.moc

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(kpanel_SOURCES) && mv messages.po ../po/kpanel.pot

install-data-local:
	if [ ! -d $(kdisknavdir) ] ; then \
		$(mkinstalldirs) $(kdisknavdir) ; \
		ln -s / $(kdisknavdir)/Root ; \
		ln -s ${KDEDIR} $(kdisknavdir)/KDE ; \
		if [ -d /cdrom ] ; then  \
			ln -s /cdrom $(kdisknavdir)/CD-ROM ; \
		elif [ -d /mnt/cdrom ] ; then  \
			ln -s /mnt/cdrom $(kdisknavdir)/CD-ROM ; \
		fi ; \
	fi

# NOTE: $(kdisknavdir) must not be removed by 'make uninstall' because it
#       might contain files and links created by the system administrator.

        
