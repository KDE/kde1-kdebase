
INCLUDES = $(all_includes) 
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD =       $(LIBJPEG) $(LIB_KDEUI) 

data_DATA = AAAAAAGeneral.ad AAAMotif.ad AAATk.ad AAAXaw.ad Editres.ad \
	Emacs.ad ML.ad Nedit.ad Netscape.ad XCalc.ad XTerm.ad XV.ad \
	XWp.ad RVPlayer.ad

datadir = $(kde_datadir)/kdisplay/app-defaults

EXTRA_DIST = $(data_DATA)

bin_PROGRAMS = krdb

krdb_SOURCES = krdb.cpp
