// This file has been created by ipcc.pl.
// (c) Torben Weis
//     weis@stud.uni-frankfurt.de

#ifndef KIOSlaveIPC_h
#define KIOSlaveIPC_h

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <qobject.h>
#include <ksock.h>

#include "kfmipc.h"


class KIOSlaveIPC : public QObject
{
    Q_OBJECT
public:
    KIOSlaveIPC( KSocket * );
    ~KIOSlaveIPC();

    void parse( char *_data, int _len );

signals:
	void hello();
private:
	void parse_hello( char *_data, int _len );
signals:
	void progress(int _percent);
private:
	void parse_progress( char *_data, int _len );
signals:
	void info(const char* _text);
private:
	void parse_info( char *_data, int _len );
signals:
	void dirEntry(const char* _dir, const char* _name, bool _isDir, int _size, const char* _date, const char* _access, const char* _owner, const char* _group);
private:
	void parse_dirEntry( char *_data, int _len );
signals:
	void data(IPCMemory _text);
private:
	void parse_data( char *_data, int _len );
signals:
	void flushDir(const char* _dir);
private:
	void parse_flushDir( char *_data, int _len );
signals:
	void done();
private:
	void parse_done( char *_data, int _len );
signals:
	void fatalError(int _error, const char* _text, int _errno);
private:
	void parse_fatalError( char *_data, int _len );
signals:
	void setPID(int _pid);
private:
	void parse_setPID( char *_data, int _len );
signals:
	void redirection(const char* _url);
private:
	void parse_redirection( char *_data, int _len );
signals:
	void mimeType(const char* _type);
private:
	void parse_mimeType( char *_data, int _len );
signals:
	void cookie(const char *_url, const char* _cookie_str);
private:
	void parse_cookie( char *_data, int _len );
public slots:
	void mount(bool _ro, const char* _fstype, const char* _dev, const char* _point);
public slots:
	void unmount(const char* _point);
public slots:
	void copy(const char* _from_url, const char* _to_url, bool _overwrite);
public slots:
	void get(const char* _url, const char* _data, const char* _cookies, bool _reload);
public slots:
	void del(const char* _url);
public slots:
	void mkdir(const char* _url);
public slots:
	void list(const char* _url, bool _bHTML);
public slots:
	void getPID();
public slots:
	void cleanUp();
public:
   int pid;

public slots:
   void closeEvent( KSocket* );
   void readEvent( KSocket* );
protected:
   KSocket* data_sock;
   char headerBuffer[11];
   int cHeader;
   bool bHeader;
   char *pBody;
   int cBody;
   int bodyLen;

public:
   /**
    * MODIFIED
    */
   time_t m_time;
  
signals:
   /**
    * MODIFIED
    */
   void closed( KIOSlaveIPC* );
};

class KIOSlaveIPCServer : public QObject
{
    Q_OBJECT
public:
    KIOSlaveIPCServer();
    ~KIOSlaveIPCServer();

signals:
   void newClient( KIOSlaveIPC * );

public slots:
   virtual void slotAccept( KSocket* );

protected:
   KServerSocket* serv_sock;
};

#endif
