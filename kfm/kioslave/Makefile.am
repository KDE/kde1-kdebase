
INCLUDES=       @all_includes@ -I..
LDADD   =       @all_libraries@ -lkdecore -lqt -lXext -lX11 @LIBSOCKET@
MOC     =       @MOC@

####### Files

bin_PROGRAMS = kioslave

kioslave_SOURCES = kioslave_ipc.cpp kioslave_ipc2.cpp main.cpp \
	ipc.cpp protocol.cpp http.cpp file.cpp ftp.cpp manage.cpp \
	../debug.cpp
noinst_HEADERS = kioslave_ipc.h main.h ipc.h protocol.h http.h \
	file.h ftp.h manage.h
BUILTSOURCES =	kioslave_ipc.moc main.moc

kioslave_ipc.cpp: kioslave_ipc.moc
main.cpp: main.moc

debug.o: ../debug.cpp ../config-kfm.h
	$(CXXCOMPILE) -c $<

####### Meta objects

kioslave_ipc.moc: kioslave_ipc.h
	$(MOC) kioslave_ipc.h -o kioslave_ipc.moc

main.moc: main.h
	$(MOC) main.h -o main.moc


