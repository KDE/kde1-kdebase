MAPS = cf.kimap csz.kimap pl.kimap rum.kimap ua.kimap uawm.kimap csp.kimap hu.kimap plnew.kimap ruw.kimap uam.kimap us.kimap csy.kimap ru.kimap ruwm.kimap uaw.kimap 

kikbd_conf = $(kde_datadir)/kikbd

# just install datas here. Use install-exec-local for scripts and etc.
# the binary itself is already installed from automake
# use mkinstalldirs, not "install -d"
# don't install a list of file. Just one file per install.
# if you have more of them, create a subdirectory with an extra Makefile 
install-data-local: 
	$(mkinstalldirs) $(kikbd_conf)
	@maps='$(MAPS)' ;\
	for map in $$maps; do \
	echo Installing $$map ;\
	$(INSTALL_DATA) $$map $(kikbd_conf); \
	done

# remove ALL you have installed in install-data-local or install-exec-local
uninstall-local:
	-rm -f $(kikbd_conf)/*.kimap






