
SUBDIRS = apps pics

install-data-local: runkappfinder_$(RUN_KAPPFINDER)

bin_SCRIPTS = kappfinder kappfinder_restart

runkappfinder_yes:
	echo "Installing non-KDE applications"
	$(SHELL) ./kappfinder $(prefix)

runkappfinder_no:

uninstall-local:
	rm -r $(kde_appsdir)/apps

EXTRA_DIST = $(bin_SCRIPTS)
