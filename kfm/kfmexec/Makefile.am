
INCLUDES=       $(all_includes) -I..
LDADD   =       $(all_libraries) -lkfm -lkdecore -lqt -lXext -lX11 $(LIBSOCKET) $(KDE_RPATH)

####### Files

bin_PROGRAMS = kfmexec

kfmexec_SOURCES = main.cpp
noinst_HEADERS = main.h
BUILT_SOURCES =	main.moc

DISTCLEANFILES = $(BUILT_SOURCES)

main.cpp: main.moc

debug.o: ../debug.cpp ../config-kfm.h
	$(CXXCOMPILE) -c ../debug.cpp

####### Meta objects

main.moc: main.h
	$(MOC) main.h -o main.moc


