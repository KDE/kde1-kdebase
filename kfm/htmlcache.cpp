#include <time.h>
#include <stdlib.h>

#include "htmlcache.h"
#include <config-kfm.h>

QString HTMLCache::cachePath;
QList<HTMLCacheJob> HTMLCache::htmlJobList;
QDict<QString> HTMLCache::urlDict;
QList<HTMLCache> HTMLCache::instanceList;
int HTMLCache::fileId = 0;

HTMLCacheJob::HTMLCacheJob( const char *_url, const char *_dest ) : KIOJob()
{
    url = _url;
    url.detach();
    dest = _dest;
    dest.detach();
    bytesTransfered = 0;
    percent = 0;
    timer1.start();
    timer2.start();
    
    connect( this, SIGNAL( finished( int ) ), this, SLOT( slotJobFinished( int ) ) );
    connect( this, SIGNAL( progress( int, int ) ), this, SLOT( slotProgress( int, int ) ) );

    timer = new QTimer( this );
    connect( timer, SIGNAL( timeout() ), this, SLOT( slotProgressTimeout() ) );
    timer->start( 5000 );
}

void HTMLCacheJob::copy()
{
    KIOJob::copy( url.data(), dest.data() );
}

HTMLCacheJob::~HTMLCacheJob()
{
}

void HTMLCacheJob::slotJobFinished( int )
{
    emit finished( this );
}

void HTMLCacheJob::slotProgressTimeout()
{
    slotProgress( percent, bytesTransfered );
}

void HTMLCacheJob::slotProgress( int _percent, int _bytesTransfered )
{
    int oldBytesTransfered = bytesTransfered;
    bool stalled = false;
    
    percent = _percent;
    bytesTransfered = _bytesTransfered;
    
    if ( bytesTransfered != oldBytesTransfered )
	timer2.restart();
    else if ( timer2.elapsed() > 5000 )
	stalled = true;
    
    emit progress( this, percent, bytesTransfered,
		   (float)bytesTransfered / 1024. / ( (float)timer1.elapsed() / 1000.0 ), stalled );
}

HTMLCache::HTMLCache()
{
    instanceList.append( this );
    
    char *p = getenv( "HOME" );
    if ( !p )
    {
	debugT("ERROR: $HOME is not defined\n");
	exit(1);
    }    
    cachePath.sprintf( "file:%s/.kfm/cache/", p );

    htmlJobList.setAutoDelete( false );
    urlDict.setAutoDelete( true );
}

void HTMLCache::slotProgress( HTMLCacheJob *_job, int _percent, int _bytesTransfered,
			      float _rate, bool _stalled )
{
    emit progress( _job->getSrcURL(), _job->getDestURL() + 5,
		   _percent, _bytesTransfered, _rate, _stalled );
}

void HTMLCache::slotURLRequest( const char *_url )
{
    QString *s = urlDict[ _url ];
    if ( s != 0L )
    {
	debugT("&&&&&&&&&&&&&& Having '%s' in cache as '%s'\n",_url,s->data());
	emit urlLoaded(  _url, s->data() );
	return;
    }

    debugT("&&&&&&&&&&&&&& Waiting for '%s'\n",_url);

    if ( waitingURLList.find( _url ) == -1 )
	waitingURLList.append( _url );
    
    // Are we waiting for this URL already ?
    HTMLCacheJob *job;
    for ( job = htmlJobList.first(); job != 0L; job = htmlJobList.next() )
	if ( strcmp( _url, job->getSrcURL() ) == 0 )
	    return;
    
    QString tmp;
    tmp.sprintf( "%s%i.%i", cachePath.data(), time( 0L ), fileId++ );
    debugT("&&&&&&&&&&&&&&&&&& New job for '%s' to '%s'\n",_url,tmp.data());
    job = new HTMLCacheJob( _url, tmp.data() );
    job->display( false );
    connect( job, SIGNAL( finished( HTMLCacheJob * ) ), this, SLOT( slotJobFinished( HTMLCacheJob * ) ) );
    connect( job, SIGNAL( progress( HTMLCacheJob *, int, int, float, bool ) ),
	     this, SLOT( slotProgress( HTMLCacheJob *, int, int, float, bool ) ) );
    htmlJobList.append( job );
    job->copy();
}

void HTMLCache::slotCancelURLRequest( const char *_url )
{
    waitingURLList.remove( _url );

    // Does anyone else wait for it ?
    HTMLCache *p;
    for ( p = instanceList.first(); p != 0L; p = instanceList.next() )
	if ( p->waitsForURL( _url ) )
	     return;

    // Find the job that works on this and kill him
    HTMLCacheJob *j;
    for ( j = htmlJobList.first(); j != 0L; j = htmlJobList.next() )
	if ( strcmp( j->getSrcURL(), _url ) == 0 )
	{
	    j->cancel();
	    htmlJobList.removeRef( j );
	    return;
	}
}

void HTMLCache::slotJobFinished( HTMLCacheJob* _job )
{
    debugT("&&&&&&&&&&&&&&&&&& URL '%s' loaded in '%s'\n",_job->getSrcURL(), _job->getDestURL() + 5 );
    // Tell all instances
    HTMLCache *p;
    for ( p = instanceList.first(); p != 0L; p = instanceList.next() )
	p->urlLoaded( _job );

    // Remove the job from the list
    htmlJobList.removeRef( _job );
}

void HTMLCache::urlLoaded( HTMLCacheJob *_job )
{    
    // Are we waiting for this URL ?
    if ( waitingURLList.find( _job->getSrcURL() ) != - 1 )
    {
      debugT("Sending signal\n");
	urlDict.insert( _job->getSrcURL(), new QString( _job->getDestURL() + 5 ) );
	debugT("Inserted\n");
	emit urlLoaded( _job->getSrcURL(), _job->getDestURL() + 5 );
	debugT("Emitted\n");
	// We dont to wait for it now any more
	waitingURLList.remove( _job->getSrcURL() );
	debugT("Removed\n");
    }
}

bool HTMLCache::waitsForURL( const char *_url )
{
    if ( waitingURLList.find( _url ) == -1 )
	return false;
    
    return true;
}
    
HTMLCache::~HTMLCache()
{
    instanceList.removeRef( this );
}

void HTMLCache::quit()
{
    HTMLCacheJob *job;
    for ( job = htmlJobList.first(); job != 0L; job = htmlJobList.next() )
	job->cancel();
}

#include "htmlcache.moc"
