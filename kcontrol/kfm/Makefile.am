
INCLUDES = $(all_includes) 
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD =       $(LIB_KDEUI) 

bin_PROGRAMS = kcmkfm
BUILT_SOURCES = htmlopts.moc miscopts.moc  kproxydlg.moc khttpoptdlg.moc useragentdlg.moc
noinst_HEADERS = htmlopts.h miscopts.h  kproxydlg.h khttpoptdlg.h useragentdlg.h

kcmkfm_SOURCES = main.cpp htmlopts.cpp miscopts.cpp  kproxydlg.cpp khttpoptdlg.cpp useragentdlg.cpp

messages:
	$(XGETTEXT) -C -ki18n -x $(includedir)/kde.pot $(kcmkfm_SOURCES) && mv messages.po ../../po/kcmkfm.pot

DISTCLEANFILES = $(BUILT_SOURCES)

htmlopts.cpp:htmlopts.moc
htmlopts.moc: htmlopts.h
	$(MOC) htmlopts.h -o htmlopts.moc

miscopts.cpp:miscopts.moc
miscopts.moc: miscopts.h
	$(MOC) miscopts.h -o miscopts.moc


kproxydlg.cpp:kproxydlg.moc
kproxydlg.moc: kproxydlg.h
	$(MOC) kproxydlg.h -o kproxydlg.moc

khttpoptdlg.cpp:khttpoptdlg.moc
khttpoptdlg.moc: khttpoptdlg.h
	$(MOC) khttpoptdlg.h -o khttpoptdlg.moc

useragentdlg.cpp:useragentdlg.moc
useragentdlg.moc: useragentdlg.h
	$(MOC) useragentdlg.h -o useragentdlg.moc
