# $Id$
#
# This has to be set. The rest should be reusable
GMOFILES = krootwm.gmo kwm.gmo kvt.gmo kcontrol.gmo kcminput.gmo \
	kdisplay.gmo kfontmanager.gmo kpanel.gmo kcmsyssound.gmo \
	kdm.gmo kcmbell.gmo kfm.gmo kfind.gmo kdehelp.gmo \
	kcminfo.gmo kcmsamba.gmo kcmlocale.gmo kmenuedit.gmo \
	kdmconfig.gmo kcmkwm.gmo kcmkpanel.gmo klock.gmo \
	kstartondesk.gmo kcmsample.gmo kcmkeys.gmo

POFILES = $(GMOFILES:.gmo=.po)

LANG = no

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

