EXTRA_DIST = mini-kdmconfig.xpm kdmconfig.xpm monitor.xpm

kde_icon_DATA = kdmconfig.xpm

picsdir = $(kde_datadir)/kdmconfig/pics
pics_DATA = monitor.xpm

install-data-local: 
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) $(srcdir)/mini-kdmconfig.xpm $(kde_minidir)/kdmconfig.xpm

uninstall-local:
	-rm -f $(kde_minidir)/kdmconfig.xpm


