data_DATA = .directory buttons.kdelnk properties.kdelnk advanced.kdelnk titlebar.kdelnk mouse.kdelnk

datadir = $(kde_appsdir)/Settings/Windows

EXTRA_DIST = $(data_DATA) kcmkwm.xpm

install-data-local: 
	$(INSTALL_DATA) kcmkwm.xpm $(kde_icondir)/kcmkwm.xpm

uninstall-local:
	rm -f $(kde_icondir)/kcmkwm.xpm

