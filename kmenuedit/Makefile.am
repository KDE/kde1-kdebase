
INCLUDES = @all_includes@ 
LDADD = @all_libraries@ -lkdecore -lkdeui -lqt -lXext -lX11 
MOC   	= @MOC@

SUBDIRS = icons

bin_PROGRAMS = kmenuedit

kmenuedit_SOURCES = kmenuedit.cpp main.cpp pmenu.cpp \
	cpopmenu.cpp confmenu.cpp button.cpp IconPathDialog.cpp \
	IconPathDialogData.cpp DesktopPathDialog.cpp \
	DesktopPathDialogData.cpp entrydialog.cpp

MOCS =	kmenuedit.moc pmenu.moc cpopmenu.moc confmenu.moc\
	button.moc IconPathDialog.moc IconPathDialogData.moc\
	DesktopPathDialog.moc DesktopPathDialogData.moc\
	entrydialog.moc

kmenuedit.cpp:kmenuedit.moc
kmenuedit.moc: kmenuedit.h
	$(MOC) kmenuedit.h -o kmenuedit.moc

pmenu.cpp:pmenu.moc
pmenu.moc: pmenu.h
	$(MOC) pmenu.h -o pmenu.moc

cpopmenu.cpp:cpopmenu.moc
cpopmenu.moc: cpopmenu.h
	$(MOC) cpopmenu.h -o cpopmenu.moc

confmenu.cpp:confmenu.moc
confmenu.moc: confmenu.h
	$(MOC) confmenu.h -o confmenu.moc

button.cpp:button.moc
button.moc: button.h
	$(MOC) button.h -o button.moc

IconPathDialog.cpp:IconPathDialog.moc IconPathDialogData.moc
IconPathDialog.moc: IconPathDialog.h
	$(MOC) IconPathDialog.h -o IconPathDialog.moc

IconPathDialogData.moc: IconPathDialogData.h
	$(MOC) IconPathDialogData.h -o IconPathDialogData.moc

DesktopPathDialog.cpp: DesktopPathDialog.moc DesktopPathDialogData.moc
DesktopPathDialog.moc: DesktopPathDialog.h
	$(MOC) DesktopPathDialog.h -o DesktopPathDialog.moc

DesktopPathDialogData.moc: DesktopPathDialogData.h
	$(MOC) DesktopPathDialogData.h -o DesktopPathDialogData.moc

entrydialog.cpp:entrydialog.moc
entrydialog.moc: entrydialog.h
	$(MOC) entrydialog.h -o entrydialog.moc


