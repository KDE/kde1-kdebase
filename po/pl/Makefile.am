
# This has to be set. The rest should be reusable
GMOFILES = kdehelp.gmo kcmdisplay.gmo kvt.gmo \
	kwm.gmo kdm.gmo kpanel.gmo kfm.gmo \
	kfontmanager.gmo kfind.gmo klock.gmo \
	krootwm.gmo kcmbell.gmo kcminfo.gmo \
	kcminput.gmo kcmkwm.gmo kcmsamba.gmo \
	kcmsample.gmo kcontrol.gmo kcmkpanel.gmo \
	kmenuedit.gmo kdmconfig.gmo kcmlocale.gmo \
	kcmsyssound.gmo kstart.gmo kikbd.gmo \
	kcmkeys.gmo kcmkfm.gmo kbgndwm.gmo \
	kpager.gmo konsole.gmo kthememgr.gmo

POFILES = $(GMOFILES:.gmo=.po)

LANG = pl

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
