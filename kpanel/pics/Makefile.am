
data_DATA = exec.xpm exit.xpm key.xpm kfm_home.xpm kpanel.xpm \
	system.xpm window_list.xpm   

datadir = $(kde_datadir)/kpanel/pics

install-data-local:
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) mini-default.xpm $(kde_minidir)

uninstall-local:
	-rm -f $(kde_minidir)/mini-default.xpm
