
# This has to be set. The rest should be reusable
DE_POFILES = de_kfind.po  de_kdehelp.po  de_kdisplay.po  de_kvt.po  de_kfm.po  de_kwm.po  de_kfontmanager.po  de_klogout.po
DE_GMFILES = de_kfind.gmo de_kdehelp.gmo de_kdisplay.gmo de_kvt.gmo de_kfm.gmo de_kwm.gmo de_kfontmanager.gmo de_klogout.gmo

NL_POFILES =  nl_kfind.po  nl_kdehelp.po  nl_kdisplay.po  nl_kvt.po  nl_kfm.po  nl_kwm.po  nl_kfontmanager.po  nl_klogout.po
NL_GMFILES = nl_kfind.gmo nl_kdehelp.gmo nl_kdisplay.gmo nl_kvt.gmo nl_kfm.gmo nl_kwm.gmo nl_kfontmanager.gmo nl_klogout.gmo  

CZ_POFILES = cz_kfind.po
CZ_GMFILES = cz_kfind.gmo

DA_POFILES = da_kdm.po
DA_GMFILES = da_kdm.gmo

FI_POFILES = fi_kfind.po
FI_GMFILES = fi_kfind.gmo

ES_POFILES = es_kfind.po  es_kdehelp.po  es_kdisplay.po  es_kvt.po  es_kfm.po  es_kwm.po  es_kfontmanager.po  es_klogout.po 
ES_GMFILES = es_kfind.gmo es_kdehelp.gmo es_kdisplay.gmo es_kvt.gmo es_kfm.gmo es_kwm.gmo es_kfontmanager.gmo es_klogout.gmo 

SV_POFILES = sv_kfind.po  sv_kdehelp.po  sv_kdisplay.po  sv_kvt.po  sv_kfm.po  sv_kwm.po  sv_kfontmanager.po  sv_klogout.po
SV_GMFILES = sv_kfind.gmo sv_kdehelp.gmo sv_kdisplay.gmo sv_kvt.gmo sv_kfm.gmo sv_kwm.gmo sv_kfontmanager.gmo sv_klogout.gmo

FR_POFILES = fr_kfind.po  fr_kdehelp.po  fr_kdisplay.po
FR_GMFILES = fr_kfind.gmo fr_kdehelp.gmo fr_kdisplay.gmo

RU_POFILES = ru_kfind.po  ru_kdehelp.po  ru_kdisplay.po  ru_kvt.po  ru_kwm.po  ru_kfontmanager.po ru_klogout.po
RU_GMFILES = ru_kfind.gmo ru_kdehelp.gmo ru_kdisplay.gmo ru_kvt.gmo ru_kwm.gmo ru_kfontmanager.gmo ru_klogout.gmo

POFILES = $(DE_POFILES) $(CZ_POFILES) $(DA_POFILES) $(ES_POFILES) $(SV_POFILES) $(FR_POFILES) $(RU_POFILES) $(NL_POFILES) $(FI_POFILES)
GMOFILES= $(DE_GMFILES) $(CZ_GMFILES) $(DA_GMFILES) $(ES_GMFILES) $(SV_GMFILES) $(FR_GMFILES) $(RU_GMFILES) $(NL_GMFILES) $(FI_GMFILES)

localedir = $(prefix)/share/locale

GMSGFMT = @GMSGFMT@
MSGFMT = @MSGFMT@
XGETTEXT = @XGETTEXT@
DISTFILES = Makefile.in $(POFILES) $(GMOFILES)


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
	lang=`echo $$cat | sed 's/_.*$$//'`; \
	name=`echo $$cat | sed -e 's/^.._//' | sed -e 's/\.gmo/\.mo/'` \
	dir=$$destdir/$$lang/LC_MESSAGES; \
	$(mkinstalldirs) $$dir; \
	$(INSTALL_DATA) $$cat $$dir/$$name; \
	  echo "installing $$cat as $$dir/$$name"; \
	done

install-no:

uninstall-local:
	@catalogs='$(GMOFILES)'; \
	for cat in $$catalogs; do \
	destdir=$(localedir); \
	lang=`echo $$cat | sed 's/_.*$$//'`; \
	name=`echo $$cat | sed -e 's/^.._//' | sed -e 's/\.gmo/\.mo/'` \
	dir=$$destdir/$$lang/LC_MESSAGES; \
	rm -f $$cat $$dir/$$name; \
	echo "removing $$dir/$$name" ; \
	done

merge:
	@catalogs='$(POFILES)'; \
	for cat in $$catalogs; do \
	name=`echo $$cat | sed -e 's/^.._//' `; \
	echo $$cat $$name; \
	msgmerge $$cat $$name > $$cat.new ; \
	if diff $$cat $$cat.new; then \
		rm $$cat.new;  \
	else  \
		mv $$cat.new $$cat ; \
	fi; \
	done

