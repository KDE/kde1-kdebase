
INCLUDES= $(all_includes) -I$(top_srcdir)/kfm
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD   = $(LIB_KIMGIO) $(LIB_KDECORE) 
CXXFLAGS= $(NOOPT_CXXFLAGS)

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

debug.o: $(top_srcdir)/kfm/debug.cpp $(top_srcdir)/kfm/config-kfm.h
	$(CXXCOMPILE) -c $(top_srcdir)/kfm/debug.cpp

####### Meta objects

kioslave_ipc.moc: kioslave_ipc.h
	$(MOC) $(srcdir)/kioslave_ipc.h -o kioslave_ipc.moc
file.moc: file.h
	$(MOC) $(srcdir)/file.h -o file.moc
ftp.moc: ftp.h
	$(MOC) $(srcdir)/ftp.h -o ftp.moc
http.moc: http.h
	$(MOC) $(srcdir)/http.h -o http.moc
tar.moc: tar.h
	$(MOC) $(srcdir)/tar.h -o tar.moc
protocol.moc: protocol.h
	$(MOC) $(srcdir)/protocol.h -o protocol.moc
subprotocol.moc: subprotocol.h
	$(MOC) $(srcdir)/subprotocol.h -o subprotocol.moc
main.moc: main.h
	$(MOC) $(srcdir)/main.h -o main.moc
cgi.moc: cgi.h
	$(MOC) $(srcdir)/cgi.h -o cgi.moc
gzip.moc: gzip.h
	$(MOC) $(srcdir)/gzip.h -o gzip.moc
icon.moc: icon.h
	$(MOC) $(srcdir)/icon.h -o icon.moc
ftp_proxy.moc: ftp_proxy.h
	$(MOC) $(srcdir)/ftp_proxy.h -o ftp_proxy.moc


