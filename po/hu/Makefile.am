
# This has to be set. The rest should be reusable
GMOFILES = kdehelp.gmo kfm.gmo kdm.gmo kpanel.gmo kcmdisplay.gmo \
	klock.gmo kwm.gmo kfind.gmo kfontmanager.gmo krootwm.gmo \
	kcmbell.gmo kcminfo.gmo kcminput.gmo kcmsamba.gmo \
	kcmsample.gmo  kvt.gmo kcmlocale.gmo kdmconfig.gmo \
	kcmkpanel.gmo kbgndwm.gmo kcmkfm.gmo kcmkwm.gmo kcmsyssound.gmo \
	kcontrol.gmo kikbd.gmo kcmkeys.gmo kmenuedit.gmo konsole.gmo \
	kpager.gmo kstart.gmo
	
POFILES = $(GMOFILES:.gmo=.po)

LANG = hu

localedir = $(kde_locale)

EXTRA_DIST = $(POFILES) 

all: all-$(USE_NLS)
install-data-local: install-$(USE_NLS)

SUFFIXES = .po .gmo

.po.gmo:
	@file=`echo $* | sed 's,.*/,,'`.gmo; \
	echo $(GMSGFMT) -o $$file $< ;\
	rm -f $$file && $(GMSGFMT) -o $$file $< 

all-yes: $(GMOFILES)
all-no:

install-yes: all
	@catalogs='$(GMOFILES)'; \
	for cat in $$catalogs; do \
	destdir=$(localedir); \
	name=`echo $$cat | sed -e 's/\.gmo/\.mo/'` \
	dir=$$destdir/$(LANG)/LC_MESSAGES; \
	$(mkinstalldirs) $$dir; \
	$(INSTALL_DATA) $$cat $$dir/$$name; \
	  echo "installing $$cat as $$dir/$$name"; \
	done

install-no:

uninstall-local:
	@catalogs='$(GMOFILES)'; \
	for cat in $$catalogs; do \
	destdir=$(localedir); \
	name=`echo $$cat | sed -e 's/\.gmo/\.mo/'` \
	dir=$$destdir/$(LANG)/LC_MESSAGES; \
	rm -f $$cat $$dir/$$name; \
	echo "removing $$dir/$$name" ; \
	done

dist-hook: $(GMOFILES)
	cp $(GMOFILES) $(distdir)
