
INCLUDES=       $(all_includes) -I..
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD   =       -lkdecore -lqt -lXext -lX11 $(LIB_KIMGIO) $(LIBSOCKET) 
CXXFLAGS= @NOOPT_CXXFLAGS@

####### Files

bin_PROGRAMS = kioslave

kioslave_SOURCES = kioslave_ipc.cpp kioslave_ipc2.cpp main.cpp xview.cpp\
	ipc.cpp protocol.cpp http.cpp file.cpp ftp.cpp manage.cpp icon.cpp \
	slave.cpp tar.cpp subprotocol.cpp cgi.cpp gzip.cpp ../debug.cpp ftp_proxy.cpp
noinst_HEADERS = kioslave_ipc.h main.h ipc.h protocol.h http.h xview.h \
	file.h ftp.h manage.h slave.h tar.h subprotocol.h cgi.h gzip.h icon.h ftp_proxy.h kio_errors.h

BUILT_SOURCES =	kioslave_ipc.moc main.moc http.moc file.moc tar.moc ftp.moc \
	protocol.moc subprotocol.moc cgi.moc gzip.moc icon.moc ftp_proxy.moc
DISTCLEANFILES = $(BUILT_SOURCES)

kioslave_ipc.cpp: kioslave_ipc.moc
main.cpp: main.moc
file.cpp: file.moc
ftp.cpp: ftp.moc
http.cpp: http.moc
tar.cpp: tar.moc
protocol.cpp: protocol.moc
subprotocol.cpp: subprotocol.moc
cgi.cpp: cgi.moc
gzip.cpp: gzip.moc
icon.cpp: icon.moc
ftp_proxy.cpp: ftp_proxy.moc

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
gzip.moc: gzip.h
	$(MOC) gzip.h -o gzip.moc
icon.moc: icon.h
	$(MOC) icon.h -o icon.moc
ftp_proxy.moc: ftp_proxy.h
	$(MOC) ftp_proxy.h -o ftp_proxy.moc


