SUBDIRS = apps pics

install-data-local: runkappfinder_$(RUN_KAPPFINDER)

install-exec-local:
	$(mkinstalldirs) $(bindir)
	$(INSTALL_SCRIPT) ./kappfinder $(bindir)
	$(INSTALL_SCRIPT) ./kappfinder_restart $(bindir)

runkappfinder_yes:
	echo "Installing non-KDE applications"
	./kappfinder $(prefix)

runkappfinder_no:

uninstall-local:
	rm -r $(kde_appsdir)/apps
