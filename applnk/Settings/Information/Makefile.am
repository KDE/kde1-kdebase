data_DATA = .directory memory.kdelnk processor.kdelnk

datadir = $(kde_appsdir)/Settings/Information

EXTRA_DIST = $(data_DATA) information.xpm

install-data-local: 
	$(INSTALL_DATA) information.xpm $(kde_icondir)/information.xpm

uninstall-local:
	rm -f $(kde_icondir)/information.xpm

