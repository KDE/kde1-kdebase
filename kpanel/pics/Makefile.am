
data_DATA = exec.xpm exit.xpm key.xpm kfm_home.xpm kpanel.xpm \
	system.xpm window_list.xpm   

datadir = $(prefix)/share/apps/kpanel/pics

install-data-local:
	$(mkinstalldirs) $(prefix)/share/icons/mini
	$(INSTALL_DATA) mini-default.xpm $(prefix)/share/icons/mini

uninstall-local:
	-rm -f $(prefix)/share/icons/mini/mini-default.xpm
