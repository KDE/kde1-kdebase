data_DATA = .directory smbstatus.kdelnk

datadir = $(kde_appsdir)/Settings/Network

EXTRA_DIST = $(data_DATA) network.xpm

install-data-local: 
	$(INSTALL_DATA) network.xpm $(kde_icondir)/network.xpm

uninstall-local:
	rm -f $(kde_icondir)/network.xpm

