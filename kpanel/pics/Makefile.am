
data_DATA = exit.xpm go.xpm key.xpm window_list.xpm shared.xpm personal.xpm pers_applnk.xpm

datadir = $(kde_datadir)/kpanel/pics

EXTRA_DIST = $(data_DATA) mini-default.xpm kdisknav.xpm

kde_icon_DATA = kdisknav.xpm
install-data-local:
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) $(srcdir)/mini-default.xpm $(kde_minidir)
	$(mkinstalldirs) $(kde_minidir)/../large
	$(INSTALL_DATA) $(srcdir)/large-go.xpm $(kde_minidir)/../large
	$(INSTALL_DATA) $(srcdir)/large-kdisknav.xpm $(kde_minidir)/../large
	$(INSTALL_DATA) $(srcdir)/large-kfm_home.xpm $(kde_minidir)/../large
	$(INSTALL_DATA) $(srcdir)/large-window_list.xpm $(kde_minidir)/../large

uninstall-local:
	-rm -f $(kde_minidir)/mini-default.xpm

SUBDIRS = mini
