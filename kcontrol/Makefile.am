SUBDIRS = sample samba bell display input kwm info

install-data-local: 
	$(mkinstalldirs) $(kde_bindir)
	$(INSTALL_PROGRAM) kcontrol $(kde_bindir)/kcontrol

uninstall-local:
	rm -f $(kde_bindir)/kcontrol
