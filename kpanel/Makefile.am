####### kpanel Makefile.am
## written by Stephan Kulow using the old Makefile by Matthias Ettrich
##

INCLUDES	= $(all_includes)
DISTCLEANFILES 	= $(BUILT_SOURCES)

CXXFLAGS = @NOOPT_CXXFLAGS@

bin_PROGRAMS = kpanel
kpanel_LDADD =	-lkfm -lkdeui -lkdecore -lqt -lXext -lX11 $(LIBSOCKET) 
LDFLAGS = $(all_libraries) $(KDE_RPATH)
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
		$(LN_S) / $(kdisknavdir)/Root ; \
		$(LN_S) ${KDEDIR} $(kdisknavdir)/KDE ; \
		for dir in /cdrom /mnt/cdrom /CDROM /CD-ROM ; do \
		  if  test -d $$dir ; then  \
			$(LN_S) $$dir $(kdisknavdir)/CD-ROM ; \
			break; \
		  fi ; \
		done \
	fi

# NOTE: $(kdisknavdir) must not be removed by 'make uninstall' because it
#       might contain files and links created by the system administrator.

        
