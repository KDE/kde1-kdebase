
INCLUDES=       $(all_includes) -I..
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD   =             $(LIB_KFM) 

####### Files

bin_PROGRAMS = kfmexec

kfmexec_SOURCES = main.cpp
noinst_HEADERS = main.h
BUILT_SOURCES =	main.moc

DISTCLEANFILES = $(BUILT_SOURCES)

main.cpp: main.moc

####### Meta objects

main.moc: main.h
	$(MOC) main.h -o main.moc


