## kdebase/Makefile.am
## (C) 1997 Stephan Kulow

SUBDIRS = kfm mimetypes kdehelp kfontmanager kfind kvt \
	kwm kpanel kwmcom kwmmodules krootwm kdm kscreensaver \
	kmenuedit kaudio kcontrol kappfinder pics applnk doc po  \
	config kstartondesk

# AUTOMAKE_OPTIONS = no-dependencies

install-data-local:
	@echo ""
	@echo ""
	@echo ""
	@echo "*************** Important *************************"
	@echo ""
	@echo " Add "$(bindir)" to your PATH"
	@echo ""
	@echo " Please report bugs with our web form at"
	@echo " http://buglist.kde.org."
	@echo ""
	@echo " Stephan Kulow <coolo@kde.org> and all the KDE"
	@echo " developers wish you fun with the KDE."
	@echo "****************************************************"
	@echo ""

acinclude.m4: acinclude.m4.in libtool.m4.in kdminclude.m4
	cat acinclude.m4.in libtool.m4.in kdminclude.m4 > acinclude.m4

EXTRA_DIST = acinclude.m4.in libtool.m4.in kdminclude.m4

messages:
	rm -rf po.backup
	mkdir po.backup
	for i in `ls -1 po/*.pot | sed -e "s#po/##"`; do \
	  grep -v "\"POT-Creation" po/$$i > po.backup/$$i ; \
	  cp po/$$i po.backup/backup_$$i ;  \
	 touch -r po/$$i po.backup/backup_$$i; \
	done
	@for subdir in $(SUBDIRS); do \
	echo "Making messages in $$subdir"; \
	  (cd $$subdir && $(MAKE) messages) \
	done 
	for  i in `ls -1 po/*.pot | sed -e "s#po/##"`; do \
	  grep -v "POT-Creation-Date:" po/$$i > temp.pot ;\
	  if diff -q temp.pot po.backup/$$i; then \
	     echo "I'm restoring $$i" ;\
	     mv po.backup/backup_$$i po/$$i; \
	   else \
	     echo "will leave $$i"; \
	fi ;\
	done
	rm -f temp.pot
	rm -rf po.backup
	(cd po && $(MAKE) merge)

