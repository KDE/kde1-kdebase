
INCLUDES = $(all_includes)
LDFLAGS =  $(all_libraries) $(KDE_RPATH)
LDADD =      $(LIB_KDEUI)

SUBDIRS = icons

bin_PROGRAMS = kmenuedit

kmenuedit_SOURCES = kmenuedit.cpp main.cpp pmenu.cpp \
	cpopmenu.cpp confmenu.cpp button.cpp IconPathDialog.cpp \
	IconPathDialogData.cpp DesktopPathDialog.cpp \
	DesktopPathDialogData.cpp entrydialog.cpp \
	MenuNameDialog.cpp MenuNameDialogData.cpp

METASOURCES =	kmenuedit.moc pmenu.moc cpopmenu.moc confmenu.moc\
	button.moc IconPathDialog.moc IconPathDialogData.moc\
	DesktopPathDialog.moc DesktopPathDialogData.moc\
	entrydialog.moc\
	MenuNameDialog.moc MenuNameDialogData.moc

noinst_HEADERS = kmenuedit.h pmenu.h MenuNameDialog.h cpopmenu.h \
		MenuNameDialogData.h confmenu.h button.h IconPathDialog.h \
		IconPathDialogData.h DesktopPathDialog.h \
		DesktopPathDialogData.h entrydialog.h

EXTRA_DIST = kmenuedit.xpm mini-kmenuedit.xpm ANNOUNCE *.dlg BUGS CHANGES \
	kmenuedit-0.2.1.lsm 

kde_icon_DATA = kmenuedit.xpm

install-data-local: 
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) $(srcdir)/mini-kmenuedit.xpm $(kde_minidir)/kmenuedit.xpm

uninstall-local:
	rm -f $(kde_minidir)/kmenuedit.xpm

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(kmenuedit_SOURCES) && mv messages.po ../po/kmenuedit.pot
