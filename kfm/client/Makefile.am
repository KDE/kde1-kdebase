
INCLUDES=       $(all_includes) -I$(srcdir)/..
LDFLAGS = 	$(all_libraries) $(KDE_RPATH)
LDADD   =       $(LIB_KFM) 

bin_PROGRAMS = kfmclient

kfmclient_SOURCES = kfmclient.cpp
noinst_HEADERS = 

kfmclient_DEPENDENCIES = debug.o

BUILT_SOURCES =

debug.o: $(top_srcdir)/kfm/debug.cpp $(top_srcdir)/kfm/config-kfm.h
	$(CXXCOMPILE) -c $(top_srcdir)/kfm/debug.cpp

####### Meta objects

