
# This has to be set. The rest should be reusable
GMOFILES = kfind.gmo kdehelp.gmo kcmdisplay.gmo kvt.gmo \
	kfm.gmo kfontmanager.gmo kcminfo.gmo kcmkpanel.gmo \
	kpanel.gmo krootwm.gmo kcmkwm.gmo kcmlocale.gmo \
	kcmbell.gmo kcminput.gmo kdm.gmo kdmconfig.gmo klock.gmo \
	kcmsamba.gmo kcmsample.gmo kcontrol.gmo kmenuedit.gmo \
	kwm.gmo kstart.gmo kcmsyssound.gmo kbgndwm.gmo kcmkeys.gmo kcmkfm.gmo \
	kpager.gmo kikbd.gmo konsole.gmo kcmdisplay.gmo

POFILES = $(GMOFILES:.gmo=.po)

LANG = ca

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
