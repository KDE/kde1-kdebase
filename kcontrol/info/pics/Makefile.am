
EXTRA_DIST = $(mini_ICONS) $(kde_icon_DATA)

mini_ICONS = mini-kcmmemory.xpm mini-kcmprocessor.xpm \
	mini-kcmdevices.xpm mini-kcmpartitions.xpm mini-kcmscsi.xpm \
	mini-kcmsound.xpm mini-kcmx.xpm

kde_icon_DATA = kcmdevices.xpm kcmmemory.xpm kcmpci.xpm kcmprocessor.xpm \
	kcmscsi.xpm kcmsound.xpm kcmx.xpm kcmpartitions.xpm

install-data-local: 
	$(mkinstalldirs) $(kde_minidir)
	@list='$(mini_ICONS)'; for p in $$list; do \
	    file2=`echo $$p | sed -e "s#mini-##"`; \
	    echo "$(INSTALL_DATA) $(srcdir)/$$p $(DESTDIR)$(kde_minidir)/$$file2"; \
	    $(INSTALL_DATA) $(srcdir)/$$p $(DESTDIR)$(kde_minidir)/$$file2; \
	done

uninstall-local:
	list='$(mini_ICONS)'; for p in $$list; do \
	    file2=`echo $$p | sed -e "s#mini-##"`; \ 
	    rm -f $(DESTDIR)$(kde_minidir)/$$file2; \
	done
