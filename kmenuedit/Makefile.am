
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

MOCS =	kmenuedit.moc pmenu.moc cpopmenu.moc confmenu.moc\
	button.moc IconPathDialog.moc IconPathDialogData.moc\
	DesktopPathDialog.moc DesktopPathDialogData.moc\
	entrydialog.moc\
	MenuNameDialog.moc MenuNameDialogData.moc

noinst_HEADERS = kmenuedit.h pmenu.h MenuNameDialog.h cpopmenu.h \
		MenuNameDialogData.h confmenu.h button.h IconPathDialog.h \
		IconPathDialogData.h DesktopPathDialog.h \
		DesktopPathDialogData.h entrydialog.h

EXTRA_DIST = kmenuedit.xpm mini-kmenuedit.xpm

DISTCLEANFILES = $(MOCS)

kmenuedit.cpp:kmenuedit.moc
kmenuedit.moc: kmenuedit.h
	$(MOC) $(srcdir)/kmenuedit.h -o kmenuedit.moc

pmenu.cpp:pmenu.moc
pmenu.moc: pmenu.h
	$(MOC) $(srcdir)/pmenu.h -o pmenu.moc

cpopmenu.cpp:cpopmenu.moc
cpopmenu.moc: cpopmenu.h
	$(MOC) $(srcdir)/cpopmenu.h -o cpopmenu.moc

confmenu.cpp:confmenu.moc
confmenu.moc: confmenu.h
	$(MOC) $(srcdir)/confmenu.h -o confmenu.moc

button.cpp:button.moc
button.moc: button.h
	$(MOC) $(srcdir)/button.h -o button.moc

IconPathDialog.cpp:IconPathDialog.moc IconPathDialogData.moc
IconPathDialog.moc: IconPathDialog.h
	$(MOC) $(srcdir)/IconPathDialog.h -o IconPathDialog.moc

IconPathDialogData.moc: IconPathDialogData.h
	$(MOC) $(srcdir)/IconPathDialogData.h -o IconPathDialogData.moc

DesktopPathDialog.cpp: DesktopPathDialog.moc DesktopPathDialogData.moc
DesktopPathDialog.moc: DesktopPathDialog.h
	$(MOC) $(srcdir)/DesktopPathDialog.h -o DesktopPathDialog.moc

DesktopPathDialogData.moc: DesktopPathDialogData.h
	$(MOC) $(srcdir)/DesktopPathDialogData.h -o DesktopPathDialogData.moc

entrydialog.cpp: entrydialog.moc
entrydialog.moc: entrydialog.h
	$(MOC) $(srcdir)/entrydialog.h -o entrydialog.moc

MenuNameDialog.cpp: MenuNameDialog.moc MenuNameDialogData.moc
MenuNameDialog.moc: MenuNameDialog.h
	$(MOC) $(srcdir)/MenuNameDialog.h -o MenuNameDialog.moc

MenuNameDialogData.moc: MenuNameDialogData.h
	$(MOC) $(srcdir)/MenuNameDialogData.h -o MenuNameDialogData.moc

kde_icon_DATA = kmenuedit.xpm

install-data-local: 
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) $(srcdir)/mini-kmenuedit.xpm $(kde_minidir)/kmenuedit.xpm

uninstall-local:
	rm -f $(kde_minidir)/kmenuedit.xpm

messages:
	$(XGETTEXT) -C -ktranslate -ki18n -x $(includedir)/kde.pot $(kmenuedit_SOURCES) && mv messages.po ../po/kmenuedit.pot
