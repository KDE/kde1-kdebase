INCLUDES = $(all_includes) 
LDADD = $(all_libraries) -lkdeui -lkdecore -lqt -lXext -lX11 $(LIBJPEG) $(LIBSOCKET) $(KDE_RPATH)

data_DATA = AAAAAAGeneral.ad AAAMotif.ad AAATk.ad AAAXaw.ad Editres.ad Emacs.ad \
ML.ad Nedit.ad Netscape.ad XCalc.ad XTerm.ad XV.ad

datadir = $(kde_datadir)/kdisplay/app-defaults

bin_PROGRAMS = krdb

krdb_SOURCES = krdb.cpp
