kdisknavdir = $(kde_datadir)/kdisknav

kdisknav_DATA = Root.kdelnk KDE.kdelnk

install-data-local:
	$(mkinstalldirs) $(kdisknavdir)
	@for dir in /cdrom /mnt/cdrom /CDROM /CD-ROM ; do \
		if test -d $$dir ; then \
			echo $(INSTALL_DATA) $(srcdir)/CD-ROM.kdelnk $(kdisknavdir) ;\
			$(INSTALL_DATA) $(srcdir)/CD-ROM.kdelnk $(kdisknavdir) ; \
			echo "URL=$$dir" >> $(kdisknavdir)/CD-ROM.kdelnk ; \
			break; \
		 fi ; \
	done 

uninstall-local:
	-rm -f $(kdisknavdir)/CD-ROM.kdelnk

EXTRA_DIST = $(kdisknav_DATA) CD-ROM.kdelnk xosview.kdelnk Calendar.kdelnk

