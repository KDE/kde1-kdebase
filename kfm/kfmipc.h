// This file has been created by ipcc.pl.
// (c) Torben Weis
//     weis@stud.uni-frankfurt.de

#ifndef ipcKFM_H
#define ipcKFM_H
#include <qobject.h>
#include <ksock.h>

struct stringList
{
    int elements;
    char **list;
};

struct intList
{
    int elements;
    int *list;
};

struct doubleList
{
    int elements;
    double *list;
};

struct charList
{
    int elements;
    char *list;
};

void write_int( int _fd, int _value );
void write_double( int _fd, double _value );
void write_char( int _fd, char _value );
void write_string( int _fd, char* _value );
void write_intList( int _fd, intList* _list );
void write_doubleList( int _fd, doubleList* _list );
void write_charList( int _fd, charList* _list );
void write_stringList( int _fd, stringList* _list );
char* read_string( char *_data, int &_pos, int _len );
int read_int( char *_data, int &_pos, int _len );
char read_char( char *_data, int &_pos, int _len );
double read_double( char *_data, int &_pos, int _len );
void read_stringList( char *_data, int &_pos, int _len, stringList *_list );
void read_intList( char *_data, int &_pos, int _len, intList *_list );
void read_doubleList( char *_data, int &_pos, int _len, doubleList *_list );
void read_charList( char *_data, int &_pos, int _len, charList *_list );

#define free_int( x ); ;
#define free_char( x ); ;
#define free_double( x ); ;

void free_string( char *_str );
void free_stringList( stringList *_list );
void free_intList( intList *_list );
void free_doubleList( doubleList *_list );
void free_charList( charList *_list );

int len_int( int _value );
int len_double( double _value );
int len_char( char _value );
int len_string( char *_str );
int len_stringList( stringList *_list );
int len_intList( intList *_list );
int len_doubleList( doubleList *_list );
int len_charList( charList *_list );

class ipcKFM : public QObject
{
	Q_OBJECT
public:
	ipcKFM( KSocket * );

	void parse( char *_data, int _len );

	void refreshDesktop();
	void parse_refreshDesktop( char *_data, int _len );
	void refreshDirectory(char* _url);
	void parse_refreshDirectory( char *_data, int _len );
	void openURL(char* _url);
	void parse_openURL( char *_data, int _len );
	void openProperties(char* _url);
	void parse_openProperties( char *_data, int _len );
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

class IPC : public QObject
{
    Q_OBJECT
public:
    IPC();
    ~IPC();

    int getPort();

public slots:
   void slotAccept( KSocket* );

protected:
   KServerSocket* serv_sock;
};

#endif
