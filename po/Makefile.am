
SUBDIRS = cz da de es fi fr it nl ru sv

all: all-@USE_NLS@
install-data-local: install-@USE_NLS@

# TODO: This does not any longer work
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

