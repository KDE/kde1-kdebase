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
    
    /**
     * Open a new URL
     */
    virtual bool browse( const char *_url, bool _reload = FALSE, bool _bHTML = FALSE,
			 const char *_currentURL = 0L, QList<KIODirectoryEntry> *list = 0L );

    /**
     * Stop the KIOJob that downloads the directory information.
     */
    virtual void stop();

    const char *getURL();
    
    /**
     * @return TRUE if the returned data is the content of a file
     *         instead of some directories content. This is
     *         very important to know if you have to create the
     *         base URL for the HTML widget.
     */
    bool isFile() { return bFileLoad; }
    
public slots:
    /**
     * Notify about new directory entries.
     * If a KIOJob is started with the command 'list', then this slot
     * is called once for every file in the directory.
     */
    void slotNewDirEntry( int _id, KIODirectoryEntry * _entry );
    void slotError( int _kioerror, const char *_text, int _errno );
    void slotFinished( int _id );
    void slotDirHTMLData( const char *_data, int _len );
    void slotData( const char* _data, int _len );
    void slotMimeType( const char *_type );
    void slotRedirection( const char *_url );
    void slotInfo( const char *_text );
   
signals:
    void data( const char *_text, int _len );
    void newDirEntry( KIODirectoryEntry *e );
    void finished();
    void mimeType( const char *_type );
    void error( int _kioerror, const char *_text );
    void info( const char *_text );
    void redirection( const char *_url );
    
protected:
    void openFile();
    void testMimeType( const char *_data, int _len );
    
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
    /**
     * This flag is set if we detected a fatal error. This keeps
     * slotFinshed away from emitting the signal @ref #finished.
     */
    bool bError;
};

#endif
