
data_DATA = exit.xpm go.xpm key.xpm window_list.xpm shared.xpm personal.xpm pers_applnk.xpm

datadir = $(kde_datadir)/kpanel/pics

EXTRA_DIST = $(data_DATA) mini-default.xpm kdisknav.xpm

install-data-local:
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) mini-default.xpm $(kde_minidir)
	$(INSTALL_DATA) kdisknav.xpm $(kde_icondir)

uninstall-local:
	-rm -f $(kde_minidir)/mini-default.xpm
	-rm -f $(kde_icondir)/kdisknav.xpm

SUBDIRS 	= mini
