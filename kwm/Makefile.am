####### kpanel Makefile.am
## written by Stephan Kulow using the old Makefile by Matthias Ettrich
##

bin_PROGRAMS 	= kwm
INCLUDES	=  $(all_includes)
LDFLAGS 	= $(all_libraries) $(KDE_RPATH)
kwm_LDADD 	= $(LIB_KFM) 
DISTCLEANFILES	= $(BUILT_SOURCES)
SUBDIRS 	= pics

#######	Files

kwm_SOURCES	= client.C manager.C main.C minicli.C warning.C drag.C logout.C taskmgr.C kURLcompletion.cpp gradientFill.C 
BUILT_SOURCES	= client.moc manager.moc main.moc minicli.moc warning.moc logout.moc taskmgr.moc kURLcompletion.moc 
noinst_HEADERS = client.h options.h manager.h main.h gradientFill.h minicli.h logout.h taskmgr.h warning.h version.h kURLcompletion.h 

EXTRA_DIST = kwm.xpm mini-kwm.xpm

client.C:client.moc
client.moc: client.h
	$(MOC) client.h -o client.moc

manager.C:manager.moc
manager.moc: manager.h
	$(MOC) manager.h -o manager.moc

main.C:main.moc
main.moc: main.h
	$(MOC) main.h -o main.moc

minicli.C:minicli.moc
minicli.moc: minicli.h
	$(MOC) minicli.h -o minicli.moc

warning.C:warning.moc
warning.moc: warning.h
	$(MOC) warning.h -o warning.moc

logout.C:logout.moc
logout.moc: logout.h
	$(MOC) logout.h -o logout.moc

taskmgr.C:taskmgr.moc
taskmgr.moc: taskmgr.h
	$(MOC) taskmgr.h -o taskmgr.moc

kURLcompletion.cpp:kURLcompletion.moc
kURLcompletion.moc: kURLcompletion.h
	$(MOC) kURLcompletion.h -o kURLcompletion.moc


messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(kwm_SOURCES) && mv messages.po ../po/kwm.pot

install-data-local: 
	$(mkinstalldirs) $(kde_icondir)
	$(INSTALL_DATA) kwm.xpm $(kde_icondir)/kwm.xpm
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) mini-kwm.xpm $(kde_minidir)/kwm.xpm

uninstall-local:
	rm -f $(kde_icondir)/kwm.xpm
	rm -f $(kde_minidir)/kwm.xpm

