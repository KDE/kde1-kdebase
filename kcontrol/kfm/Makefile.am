
INCLUDES = $(all_includes) 
LDFLAGS =  $(all_libraries) $(KDE_RPATH)
LDADD =    $(LIB_KDEUI) 

bin_PROGRAMS = kcmkfm
METASOURCES = htmlopts.moc miscopts.moc  kproxydlg.moc khttpoptdlg.moc \
	useragentdlg.moc kcookiesdlg.moc rootopts.moc
noinst_HEADERS = htmlopts.h miscopts.h  kproxydlg.h khttpoptdlg.h \
	useragentdlg.h kcookiesdlg.h rootopts.h

kcmkfm_SOURCES = main.cpp htmlopts.cpp miscopts.cpp  kproxydlg.cpp khttpoptdlg.cpp \
	useragentdlg.cpp kcookiesdlg.cpp rootopts.cpp

messages:
	$(XGETTEXT) -C -ki18n -x $(includedir)/kde.pot $(kcmkfm_SOURCES) && mv messages.po ../../po/kcmkfm.pot

