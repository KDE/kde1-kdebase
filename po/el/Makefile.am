
# This has to be set. The rest should be reusable
GMOFILES = kcmdisplay.gmo kfm.gmo kfind.gmo kpanel.gmo kwm.gmo \
	kmenuedit.gmo kcmsyssound.gmo kcminput.gmo kcminfo.gmo \
	kcmkpanel.gmo kcmkwm.gmo kcmlocale.gmo kcmsamba.gmo \
	kcontrol.gmo kdehelp.gmo kdmconfig.gmo kfontmanager.gmo \
	klock.gmo krootwm.gmo kvt.gmo kdm.gmo kcmbell.gmo  \
	kcmsample.gmo

POFILES = $(GMOFILES:.gmo=.po)

LANG = el

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

