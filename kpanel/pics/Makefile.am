
data_DATA = exit.xpm go.xpm key.xpm window_list.xpm shared.xpm personal.xpm pers_applnk.xpm

datadir = $(kde_datadir)/kpanel/pics

EXTRA_DIST = $(data_DATA) mini-default.xpm kdisknav.xpm

kde_icon_DATA = kdisknav.xpm
install-data-local:
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) $(srcdir)/mini-default.xpm $(kde_minidir)
	$(mkinstalldirs) $(datadir)/large
	$(INSTALL_DATA) $(srcdir)/large-go.xpm $(datadir)/large/go.xpm
	$(INSTALL_DATA) $(srcdir)/large-kdisknav.xpm $(datadir)/large/kdisknav.xpm
	$(INSTALL_DATA) $(srcdir)/large-kfm_home.xpm $(datadir)/large/kfm_home.xpm
	$(INSTALL_DATA) $(srcdir)/large-window_list.xpm $(datadir)/large/window_list.xpm
	$(INSTALL_DATA) $(srcdir)/large-pers_applnk.xpm $(datadir)/large/pers_applnk.xpm

uninstall-local:
	rm -f $(datadir)/large/go.xpm
	rm -f $(datadir)/large/kdisknav.xpm
	rm -f $(datadir)/large/kfm_home.xpm
	rm -f $(datadir)/large/window_list.xpm
	rm -f $(kde_minidir)/mini-default.xpm

SUBDIRS = mini
