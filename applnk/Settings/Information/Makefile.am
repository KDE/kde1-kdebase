data_DATA = .directory memory.kdelnk processor.kdelnk dma.kdelnk interrupts.kdelnk ioports.kdelnk pci.kdelnk sound.kdelnk devices.kdelnk scsi.kdelnk partitions.kdelnk xserver.kdelnk

datadir = $(kde_appsdir)/Settings/Information

EXTRA_DIST = $(data_DATA) information.xpm

install-data-local: 
	$(INSTALL_DATA) information.xpm $(kde_icondir)/information.xpm

uninstall-local:
	rm -f $(kde_icondir)/information.xpm

