
INCLUDES= $(all_includes) -I$(top_srcdir)/kfm
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD   = $(LIB_KIMGIO) $(LIB_KDECORE) $(LIBVOLMGT)
CXXFLAGS= $(NOOPT_CXXFLAGS)

####### Files

bin_PROGRAMS = kioslave

kioslave_SOURCES = kioslave_ipc.cpp kioslave_ipc2.cpp main.cpp xview.cpp\
	ipc.cpp protocol.cpp http.cpp file.cpp ftp.cpp manage.cpp icon.cpp \
	slave.cpp tar.cpp subprotocol.cpp cgi.cpp gzip.cpp ftp_proxy.cpp
noinst_HEADERS = kioslave_ipc.h main.h ipc.h protocol.h http.h xview.h \
	file.h ftp.h manage.h slave.h tar.h subprotocol.h cgi.h gzip.h icon.h ftp_proxy.h kio_errors.h

METASOURCES =	kioslave_ipc.moc main.moc http.moc file.moc tar.moc ftp.moc \
	protocol.moc subprotocol.moc cgi.moc gzip.moc icon.moc ftp_proxy.moc

debug.o: $(top_srcdir)/kfm/debug.cpp $(top_srcdir)/kfm/config-kfm.h
	$(CXXCOMPILE) -c $(top_srcdir)/kfm/debug.cpp

kioslave_DEPENDENCIES = debug.o

