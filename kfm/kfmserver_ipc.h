// This file has been created by ipcc.pl.
// (c) Torben Weis
//     weis@stud.uni-frankfurt.de

#ifndef KfmServerIpc_h
#define KfmServerIpc_h
#include <qobject.h>
#include <ksock.h>
#include <ctype.h>
#include "kfmipc.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

class KfmServIpc : public QObject
{
    Q_OBJECT
public:
    KfmServIpc( KSocket * );
    ~KfmServIpc();

    void parse( char *_data, int _len );

signals:
	void refreshDesktop();
private:
	void parse_refreshDesktop( char *_data, int _len );
signals:
	void refreshDirectory(const char* _url);
private:
	void parse_refreshDirectory( char *_data, int _len );
signals:
	void openURL(const char* _url);
private:
	void parse_openURL( char *_data, int _len );
signals:
	void openProperties(const char* _url);
private:
	void parse_openProperties( char *_data, int _len );
signals:
	void list(const char* _url);
private:
	void parse_list( char *_data, int _len );
signals:
	void exec(const char* _url, const char* _binding);
private:
	void parse_exec( char *_data, int _len );
signals:
	void copy(const char* _src, const char* _dest);
private:
	void parse_copy( char *_data, int _len );
signals:
	void move(const char* _src, const char* _dest);
private:
	void parse_move( char *_data, int _len );
signals:
	void moveClient(const char* _src, const char* _dest);
private:
	void parse_moveClient( char *_data, int _len );
signals:
	void copyClient(const char* _src, const char* _dest);
private:
	void parse_copyClient( char *_data, int _len );
signals:
	void sortDesktop();
private:
	void parse_sortDesktop( char *_data, int _len );
signals:
	void configure();
private:
	void parse_configure( char *_data, int _len );
signals:
	void auth(const char* _password);
private:
	void parse_auth( char *_data, int _len );
signals:
	void selectRootIcons(int _x, int _y, int _w, int _h, bool _add);
private:
	void parse_selectRootIcons( char *_data, int _len );
public slots:
	void finished();
public slots:
	void error(int _kerror, const char* _text);
public slots:
	void dirEntry(const char* _name, const char* _access, const char* _owner, const char* _group, const char* _date, int _size);

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
};

class KfmIpcServer : public QObject
{
    Q_OBJECT
public:
    KfmIpcServer();
    ~KfmIpcServer();

signals:
   void newClient( KfmServIpc * );

public slots:
   virtual void slotAccept( KSocket* );

protected:
   KServerSocket* serv_sock;
};

#endif
