#ifndef _kfmjob_h
#define _kfmjob_h

#include <qobject.h>
#include <qstring.h>

#include <stdio.h>
#include <stdlib.h>

#include "kioserver.h"
#include "kiojob.h"
#include "kmimemagic.h"

class KFMJob : public QObject
{
    Q_OBJECT
public:
    KFMJob();
    ~KFMJob();
    
    /// Open a new URL
    virtual bool browse( const char *_url, bool _reload = FALSE, bool _bHTML = FALSE,
			 const char *_currentURL = 0L, QList<KIODirectoryEntry> *list = 0L );

    /// Stop the KIOJob that downloads the directory information
    virtual void stop();

    const char *getURL();
        
public slots:
    /// Notify about new directory entries
    /**
      If a KIOJob is started with the command 'list', then this slot
      is called once for every file in the directory.
      */
    void slotNewDirEntry( int _id, KIODirectoryEntry * _entry );
    void slotError( const char *_text );
    void slotFinished( int _id );
    void slotDirHTMLData( const char *_data );
    void slotData( const char* _data );
    void slotMimeType( const char *_type );
    void slotRedirection( const char *_url );
    void slotInfo( const char *_text );
   
signals:
    void data( const char *_text );
    void newDirEntry( KIODirectoryEntry *e );
    void finished();
    void mimeType( const char *_type );
    void error( const char *_text );
    void info( const char *_text );
    
protected:
    void openFile();
    void testMimeType( const char *_data );
    
    KIOJob *job;
    FILE *f;
    bool isHTML;
    bool isDir;
    bool bRunning;
    bool bFinished;
    bool bCheckedMimeType;
    bool bFileLoad;
    QString tmpFile;
    QString url;
    int bytesRead;
    QString dataBuffer;
};

#endif
