
# This has to be set. The rest should be reusable
GMOFILES = kfm.gmo kdehelp.gmo kcmdisplay.gmo kdm.gmo kfind.gmo \
	kfontmanager.gmo klock.gmo kpanel.gmo \
	kvt.gmo kwm.gmo kcminfo.gmo kcminput.gmo kcmkwm.gmo kcmkpanel.gmo \
	kcmsamba.gmo kcmsample.gmo kcontrol.gmo kdmconfig.gmo krootwm.gmo \
	kcmlocale.gmo kmenuedit.gmo kcmbell.gmo kcmsyssound.gmo \
	kstart.gmo kikbd.gmo kcmkeys.gmo kcmkfm.gmo

POFILES = $(GMOFILES:.gmo=.po)

LANG = pt

localedir = $(kde_locale)

DISTFILES = Makefile.am Makefile.in $(POFILES) $(GMOFILES)

all: all-$(USE_NLS)
install-data-local: install-$(USE_NLS)

SUFFIXES = .po .gmo

.po.mo:
	$(MSGFMT) -o $@ $<

.po.gmo:
	file=$(srcdir)/`echo $* | sed 's,.*/,,'`.gmo \
	&& rm -f $$file && $(GMSGFMT) -o $$file $< 

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

