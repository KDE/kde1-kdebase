#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stddef.h>
#include <sys/stat.h>

#include <qmsgbox.h>

#include "kfmjob.h"
#include "kbind.h"
#include <config-kfm.h>
#include "kmsgwin.h"
#include "kfmdlg.h"
#include "kmimemagic.h"

#include <klocale.h>

int tmpFileCounter = 0;

KFMJob::KFMJob( )
{
    job = 0L;    
}

bool KFMJob::browse( const char *_url, bool _reload, bool _bHTML, const char *_currentURL, QList<KIODirectoryEntry> *_list )
{
    debugT("Changing to ftp %s\n", _url );
    
    KURL u( _url );
    if ( u.isMalformed() )
    {
	warning(klocale->translate("ERROR: Malformed URL"));
	return false;
    }

    url = _url;

    if ( job )
	delete job;	
    job = 0L;

    dataBuffer = "";
    
    // We are not shure yet
    isDir = FALSE;
    isHTML = FALSE;
    bCheckedMimeType = FALSE;
    bFinished = FALSE;
    bRunning = TRUE;
    f = 0L;
    
    bool isFile = FALSE;
    // Lets have a look at what we already know about the URL.
    if ( _currentURL && _list && strncmp( _currentURL, _url, strlen( _currentURL ) ) == 0L )
    {
	// Get the current directory
	KURL u2( _currentURL );
	QString d = u2.directoryURL();
	
	KIODirectoryEntry *e;
	for ( e = _list->first(); e != 0L; e = _list->next() )
	{
	    QString str = d.data();
	    str += e->getName();
	    debugT("COMPARING '%s' '%s'\n",str.data(),_url );
	    if ( str == _url )
		isFile = e->isFile();
	}
    }
    
    // A HACK
    // Now we clear the list since it may happen that we emit 'newDirEntry' before we
    // return.
    if ( _list )
	_list->clear();
    
    // Can we get a directory listing with the protocol or do we know that it is a file ?
    if ( KIOServer::getKIOServer()->supports( _url, KIO_List ) && !isFile )
    {
	if ( url.right( 1 ) != "/" )
	    url += "/"; 

	// Lets try to load it as directory
	printf("Loading DIRECTORY\n");    
	bFileLoad = FALSE;
	job = new KIOJob;
	job->setAutoDelete( FALSE );
	job->display( false );
	connect( job, SIGNAL( error( const char* ) ), this, SLOT( slotError( const char* ) ) );
	connect( job, SIGNAL( newDirEntry( int, KIODirectoryEntry* ) ),
		 this, SLOT( slotNewDirEntry( int, KIODirectoryEntry* ) ) );
	connect( job, SIGNAL( finished( int ) ), this, SLOT( slotFinished( int ) ) );
	connect( job, SIGNAL( data( const char* ) ), this, SLOT( slotDirHTMLData( const char* ) ) );
	connect( job, SIGNAL( redirection( const char* ) ), this, SLOT( slotRedirection( const char* ) ) );
	connect( job, SIGNAL( info( const char* ) ), this, SLOT( slotInfo( const char* ) ) );

	// If we get HTML, then slotDirHTMLData will receive the HTML code.
	job->list( url, _reload, _bHTML );
    }
    else
	openFile();
    
    return true;
}

void KFMJob::openFile()
{
    printf("OPEN FILE\n");
    isDir = FALSE;
    
    // OK, we try to load the file
    job = new KIOJob;
    job->setAutoDelete( FALSE );
    job->display( false );
    connect( job, SIGNAL( finished( int ) ), this, SLOT( slotFinished( int ) ) );
    connect( job, SIGNAL( error( const char* ) ), this, SLOT( slotError( const char* ) ) );
    connect( job, SIGNAL( data( const char* ) ), this, SLOT( slotData( const char* ) ) );
    connect( job, SIGNAL( mimeType( const char* ) ), this, SLOT( slotMimeType( const char* ) ) );
    connect( job, SIGNAL( redirection( const char* ) ), this, SLOT( slotRedirection( const char* ) ) );
    connect( job, SIGNAL( info( const char* ) ), this, SLOT( slotInfo( const char* ) ) );
    
    // Delete the trailing / since we assume that the URL is a file
    if ( url.right(1) == "/" )
	url = url.left( url.length() - 1 );
    
    bFileLoad = TRUE;
    job->get( url );
}

void KFMJob::slotRedirection( const char *_url )
{
    url = _url;
}

void KFMJob::slotInfo( const char *_text )
{
    emit info( _text );
}

void KFMJob::slotDirHTMLData( const char *_data )
{
    if ( !bCheckedMimeType )
    {
	// This slot is called due to "job->list".
	// If we get a call we can assume that the URL is indeed a directory.
	isDir = TRUE;
	emit mimeType( "text/html" );
	bCheckedMimeType = TRUE;
    }
    
    emit data( _data );
}

void KFMJob::slotError( const char *_text )
{
    QMessageBox::message( klocale->translate("KFM Error"),
			  _text );
    emit error( _text );
    
    /*
    KMsgWin *m = new KMsgWin( 0L, klocale->translate("Error"), 
			      _text, KMsgWin::EXCLAMATION, 
			      klocale->translate("Close") );
    m->exec(); */
}

void KFMJob::slotNewDirEntry( int , KIODirectoryEntry * _entry )
{
    isDir = TRUE;
    emit newDirEntry( _entry );
}

void KFMJob::slotMimeType( const char *_type )
{
    bCheckedMimeType = TRUE;
    emit mimeType( _type );
}

void KFMJob::slotData( const char *_text )
{
    if ( !bCheckedMimeType )
    {
	dataBuffer += _text;
	if ( dataBuffer.length() )
	    testMimeType( dataBuffer );
	return;
    }
    
    emit data( _text );
}

void KFMJob::testMimeType( const char *_text)
{
    bCheckedMimeType = TRUE;
    
    debugT("SAMPLE:\n%s\n",_text);
    
    KMimeMagicResult *result = KMimeType::findBufferType( _text, strlen( _text ) );
    debugT("RETURN '%s' '%i'\n",result->getContent().data(),result->getAccuracy() );
    
    if ( strcmp( "text/html", result->getContent() ) == 0 ) 
    {
	debugT("IS HTML\n");
	isHTML = TRUE;
	emit mimeType( "text/html" );
	emit data( _text );
    }
    else if ( result->getAccuracy() > 40 )
    {
	emit mimeType( result->getContent() );
    }
    else
    {
	debugT("UNKNOWN TYPE\n");
	// Dont know the mime type
	emit mimeType( 0L );
    }
}

void KFMJob::slotFinished( int )
{
    bFinished = TRUE;
    
    // Did we only try to load the URL as directory and did we get 0 entries ?
    if ( !bFileLoad && !isDir )
    {
	disconnect( job, 0, this, 0 );
	job->setAutoDelete( TRUE );
	job = 0L;
	// Try to open as file
	openFile();
	return;
    }

    // Are we loading a file ?
    if ( bFileLoad )
    {
	// Did we check for the mime type already ?
	if ( !bCheckedMimeType )
	{
	    testMimeType( dataBuffer );
	}
    }
    
    disconnect( job, 0, this, 0 );
    job->setAutoDelete( TRUE );
    job = 0L;

    emit finished();

    bRunning = FALSE;
}

void KFMJob::stop()
{
    if ( !bRunning )
	return;
    
    bRunning = FALSE;
   
    printf("Stopping\n");
    
    if ( job )
    {
	disconnect( job, 0, this, 0 );
	job->setAutoDelete( TRUE );
	if ( !bFinished )
	    job->cancel();
	job = 0L;
    }

    bFinished = TRUE;
}

const char* KFMJob::getURL()
{
    return url;
}

KFMJob::~KFMJob()
{
    stop();
}


#include "kfmjob.moc"
