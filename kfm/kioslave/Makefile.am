
INCLUDES=       @all_includes@ -I..
LDADD   =       @all_libraries@ -lkdecore -lqt -lXext -lX11 @LIBSOCKET@
MOC     =       @MOC@

####### Files

bin_PROGRAMS = kioslave

kioslave_SOURCES = kioslave_ipc.cpp kioslave_ipc2.cpp main.cpp \
	ipc.cpp protocol.cpp http.cpp file.cpp ftp.cpp manage.cpp \
	slave.cpp tar.cpp subprotocol.cpp cgi.cpp ../debug.cpp
noinst_HEADERS = kioslave_ipc.h main.h ipc.h protocol.h http.h \
	file.h ftp.h manage.h slave.h tar.h subprotocol.h cgi.h
BUILTSOURCES =	kioslave_ipc.moc main.moc http.moc file.moc tar.moc ftp.moc protocol.moc subprotocol.h cgi.moc

kioslave_ipc.cpp: kioslave_ipc.moc
main.cpp: main.moc
file.cpp: file.moc
ftp.cpp: ftp.moc
http.cpp: http.moc
tar.cpp: tar.moc
protocol.cpp: protocol.moc
subprotocol.cpp: subprotocol.moc
cgi.cpp: cgi.moc

debug.o: ../debug.cpp ../config-kfm.h
	$(CXXCOMPILE) -c ../debug.cpp

####### Meta objects

kioslave_ipc.moc: kioslave_ipc.h
	$(MOC) kioslave_ipc.h -o kioslave_ipc.moc
file.moc: file.h
	$(MOC) file.h -o file.moc
ftp.moc: ftp.h
	$(MOC) ftp.h -o ftp.moc
http.moc: http.h
	$(MOC) http.h -o http.moc
tar.moc: tar.h
	$(MOC) tar.h -o tar.moc
protocol.moc: protocol.h
	$(MOC) protocol.h -o protocol.moc
subprotocol.moc: subprotocol.h
	$(MOC) subprotocol.h -o subprotocol.moc
main.moc: main.h
	$(MOC) main.h -o main.moc
cgi.moc: cgi.h
	$(MOC) cgi.h -o cgi.moc


