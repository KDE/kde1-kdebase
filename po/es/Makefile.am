
# This has to be set. The rest should be reusable
GMOFILES = kfind.gmo kdehelp.gmo kcmdisplay.gmo kvt.gmo \
	kfm.gmo kwm.gmo kfontmanager.gmo \
	kdm.gmo klock.gmo kpanel.gmo kcminfo.gmo kcmbell.gmo \
	kcminput.gmo krootwm.gmo kcmkwm.gmo kcontrol.gmo \
	kdmconfig.gmo kcmsamba.gmo kmenuedit.gmo kcmlocale.gmo \
	kcmkpanel.gmo kcmsyssound.gmo kstart.gmo kikbd.gmo kpager.gmo \
	kcmkeys.gmo kcmkfm.gmo kcmsample.gmo konsole.gmo
POFILES = $(GMOFILES:.gmo=.po)

LANG = es

localedir = $(kde_locale)

DISTFILES = Makefile.am Makefile.in $(POFILES) $(GMOFILES)

all: all-@USE_NLS@
install-data-local: install-@USE_NLS@

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

