
EXTRA_DIST = $(mini_ICONS) $(kde_icon_DATA)

mini_ICONS = mini-kcmmemory.xpm mini-kcmprocessor.xpm \
	mini-kcmdevices.xpm mini-kcmpartitions.xpm mini-kcmscsi.xpm \
	mini-kcmsound.xpm mini-kcmx.xpm

kde_icon_DATA = kcmdevices.xpm kcmmemory.xpm kcmpci.xpm kcmprocessor.xpm \
	kcmscsi.xpm kcmsound.xpm kcmx.xpm 

install-data-local: 
	$(mkinstalldirs) $(kde_minidir)
	@list='$(mini_ICONS)'; for p in $$list; do \
	    echo "$(INSTALL_DATA) $(srcdir)/$$p $(DESTDIR)$(kde_minidir)/$$p"; \
	    $(INSTALL_DATA) $(srcdir)/$$p $(DESTDIR)$(kde_minidir)/$$p; \
	done

uninstall-local:
	list='$(mini_ICONS)'; for p in $$list; do \
	    rm -f $(DESTDIR)$(kde_minidir)/$$p; \
	done
