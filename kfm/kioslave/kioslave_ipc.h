// This file has been created by ipcc.pl.
// (c) Torben Weis
//     weis@stud.uni-frankfurt.de

#ifndef KIOSlaveIPC_h
#define KIOSlaveIPC_h

#include <ctype.h>
#include <ksock.h>
#include <qobject.h>
#include "ipc.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

class KIOSlaveIPC : public QObject
{
    Q_OBJECT
public:
    KIOSlaveIPC( int _port );
    ~KIOSlaveIPC();

    bool isConnected();

public slots:
	void hello();
public slots:
	void progress(int _percent);
public slots:
	void info(const char* _text);
public slots:
	void dirEntry(const char* _dir, const char* _name, bool _isDir, int _size, const char* _date, const char* _access, const char* _owner, const char* _group);
public slots:
	void data(const char* _text);
public slots:
	void flushDir(const char* _dir);
public slots:
	void done();
public slots:
	void fatalError(int _error, const char* _text, int _errno);
public slots:
	void setPID(int _pid);
public slots:
	void redirection(const char* _url);
public slots:
	void mimeType(const char* _type);
signals:
	void mount(bool _ro, const char* _fstype, const char* _dev, const char* _point);
private:
	void parse_mount( char *_data, int _len );
signals:
	void unmount(const char* _point);
private:
	void parse_unmount( char *_data, int _len );
signals:
	void copy(const char* _from_url, const char* _to_url, bool _overwrite);
private:
	void parse_copy( char *_data, int _len );
signals:
	void get(const char* _url);
private:
	void parse_get( char *_data, int _len );
signals:
	void del(const char* _url);
private:
	void parse_del( char *_data, int _len );
signals:
	void mkdir(const char* _url);
private:
	void parse_mkdir( char *_data, int _len );
signals:
	void list(const char* _url, bool _bHTML);
private:
	void parse_list( char *_data, int _len );
signals:
	void getPID();
private:
	void parse_getPID( char *_data, int _len );
signals:
	void cleanUp();
private:
	void parse_cleanUp( char *_data, int _len );
public slots:
    void readEvent( KSocket * );
    void closeEvent( KSocket * );
private:
    void parse( char *_data, int _len );

    int port;
    KSocket *sock;
    bool connected;
    char headerBuffer[11];
    int cHeader;
    bool bHeader;
    char *pBody;
    int cBody;
    int bodyLen;
};

#endif
