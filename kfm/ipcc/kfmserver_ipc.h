// This file has been created by ipcc.pl.
// (c) Torben Weis
//     weis@stud.uni-frankfurt.de

#ifndef KfmIpc_h
#define KfmIpc_h
#include <qobject.h>
#include <ksock.h>
#include <ctype.h>
#include "ipc.h"

class KfmIpc : public QObject
{
    Q_OBJECT
public:
    KfmIpc( KSocket * );
    ~KfmIpc();

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
	void ask(int _x, int _y, const char* _src, const char* _dest);
private:
	void parse_ask( char *_data, int _len );
signals:
	void sortDesktop();
private:
	void parse_sortDesktop( char *_data, int _len );
public slots:
	void finished();

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

    int getPort();

signals:
   void newClient( KfmIpc * );

public slots:
   virtual void slotAccept( KSocket* );

protected:
   KServerSocket* serv_sock;
};

#endif
