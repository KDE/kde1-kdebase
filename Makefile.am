## kdebase/Makefile.am
## (C) 1997 Stephan Kulow

SUBDIRS = kfm mimetypes kdehelp kfontmanager kfind kvt \
	kwm kpanel kwmcom kwmmodules krootwm kdm kscreensaver \
	kmenuedit kaudio kcontrol kappfinder pics applnk doc po 

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

#acinclude.m4: acinclude.m4.in libtool.m4.in kdminclude.m4
#	cat acinclude.m4.in libtool.m4.in kdminclude.m4 > acinclude.m4

messages:
	@for subdir in $(SUBDIRS); do \
	echo "Making messages in $$subdir"; \
	  (cd $$subdir && $(MAKE) messages) \
	done 
	(cd po && $(MAKE) merge)
