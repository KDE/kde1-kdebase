
# This has to be set. The rest should be reusable
GMOFILES = kfm.gmo kcmdisplay.gmo kwm.gmo kfind.gmo kdehelp.gmo \
	kdm.gmo kfontmanager.gmo kpanel.gmo kvt.gmo \
	kcmkpanel.gmo kcontrol.gmo kcmlocale.gmo kcmbell.gmo \
	kcminfo.gmo kcminput.gmo krootwm.gmo kcmsamba.gmo kcmkwm.gmo \
	kcmsyssound.gmo kmenuedit.gmo kdmconfig.gmo klock.gmo \
	kstart.gmo kcmkfm.gmo kcmsample.gmo kcmkeys.gmo \
	kikbd.gmo konsole.gmo kpager.gmo kbgndwm.gmo

POFILES = $(GMOFILES:.gmo=.po)

LANG = it

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
