## kdebase/Makefile.am
## (C) 1997 Stephan Kulow

SUBDIRS = kcheckpass kfm mimetypes kdehelp kfontmanager kfind kvt \
	kwm kpanel kwmcom kwmmodules krootwm kdm kscreensaver \
	kmenuedit kaudio kcontrol kappfinder pics applnk doc config\
	kstart kikbd konsole po

AUTOMAKE_OPTIONS = foreign 1.4

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

$(top_srcdir)/acinclude.m4: $(top_srcdir)/admin/acinclude.m4.in $(top_srcdir)/kdminclude.m4 $(top_srcdir)/admin/libtool.m4.in
	@cd $(top_srcdir) && cat admin/acinclude.m4.in kdminclude.m4 admin/libtool.m4.in > acinclude.m4

messages:
	rm -rf po.backup
	mkdir po.backup
	for i in `ls -1 po/*.pot | sed -e "s#po/##"`; do \
	  grep -v "\"POT-Creation" po/$$i > po.backup/$$i ; \
	  cp po/$$i po.backup/backup_$$i ;  \
	done
	files=`ls -1`; \
	dirs=`for i in $$files; do if test -d $$i && test -r $$i/Makefile.am; then echo $$i; fi; done`; \
	for subdir in $$dirs; do \
	  echo "Making messages in $$subdir"; \
	  (cd $$subdir && $(MAKE) -f Makefile.am messages) \
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
	(cd po && $(MAKE) -f Makefile.am merge)

EXTRA_DIST = admin bsd-port debian kdebase.spec.in README.pam kde.pamd \
	kdminclude.m4 ltconfig.patch-1

dist-hook:
	cd $(top_distdir) && perl $(top_srcdir)/admin/automoc -padmin

