####### kpanel Makefile.am
## written by Stephan Kulow using the old Makefile by Matthias Ettrich
##

INCLUDES	= $(all_includes)
DISTCLEANFILES 	= $(BUILT_SOURCES)

CXXFLAGS = $(NOOPT_CXXFLAGS)

bin_PROGRAMS = kpanel
kpanel_LDADD =	$(LIB_KFM) $(LIB_KDEUI) 
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
		for f in Root KDE ; do \
			$(INSTALL_DATA) sample_kdelnk_files/$$f.kdelnk $(kdisknavdir)/$$f.kdelnk ; \
		done ; \
		for dir in /cdrom /mnt/cdrom /CDROM /CD-ROM ; do \
		  if  test -d $$dir ; then \
			cp sample_kdelnk_files/CD-ROM.kdelnk $(kdisknavdir)/. ; \
			echo "URL=$$dir" >> $(kdisknavdir)/CD-ROM.kdelnk ; \
			break; \
		  fi ; \
		done \
	fi
	$(mkinstalldirs) $(kde_datadir)/kpanel
	$(mkinstalldirs) $(kde_datadir)/kpanel/default
	$(INSTALL_DATA) personal.kdelnk $(kde_datadir)/kpanel/default/personal_directory
	$(INSTALL_DATA) Home.kdelnk $(kde_datadir)/kpanel/default/Home.kdelnk
	$(INSTALL_DATA) Desktop.kdelnk $(kde_datadir)/kpanel/default/Desktop.kdelnk

# NOTE: $(kdisknavdir) must not be removed by 'make uninstall' because it
#       might contain files and links created by the system administrator.

        
