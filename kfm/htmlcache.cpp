#include <time.h>
#include <stdlib.h>

#include "htmlcache.h"
#include <config-kfm.h>

QString HTMLCache::cachePath;
QList<HTMLCacheJob> HTMLCache::staticJobList;
QDict<QString> HTMLCache::urlDict;
QList<HTMLCache> HTMLCache::instanceList;
int HTMLCache::fileId = 0;

HTMLCacheJob::HTMLCacheJob( const char *_url, const char *_dest ) : KIOJob()
{
    url = _url;
    url.detach();
    dest = _dest;
    dest.detach();
    
    connect( this, SIGNAL( finished( int ) ), this, SLOT( slotJobFinished( int ) ) );
    connect( this, SIGNAL( error( int, const char* ) ), this, SLOT( slotError( int, const char* ) ) );
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

void HTMLCacheJob::slotError( int _kioerror, const char* _text )
{
    emit error( this, _kioerror, _text );
}

HTMLCache::HTMLCache()
{
    instanceList.append( this );
    
    char *p = getenv( "HOME" );
    if ( !p )
    {
	warning("ERROR: $HOME is not defined\n");
	exit(1);
    }    
    cachePath.sprintf( "file:%s/.kde/kfm/cache/", p );

    staticJobList.setAutoDelete( false );
    urlDict.setAutoDelete( true );
    todoURLList.setAutoDelete( TRUE );
    instanceJobList.setAutoDelete( false );
}

void HTMLCache::slotURLRequest( const char *_url )
{
    // Is the URL already cached ?
    QString *s = urlDict[ _url ];
    if ( s != 0L )
    {
	emit urlLoaded(  _url, s->data() );
	return;
    }
    
    // Are we waiting for this URL already ?
    // We can see all jobs of every instance here.
    HTMLCacheJob *job;
    for ( job = staticJobList.first(); job != 0L; job = staticJobList.next() )
	if ( strcmp( _url, job->getSrcURL() ) == 0 )
	    return;

    // Add it to the list of urls this instance is waiting for
    waitingURLList.append( _url );
    
    // Can we serve this job yet ?
    if ( instanceJobList.count() == MAX_JOBS )
    {
	todoURLList.append( _url );
	return;
    }
    
    QString tmp;
    tmp.sprintf( "%s%i.%i", cachePath.data(), time( 0L ), fileId++ );
    job = new HTMLCacheJob( _url, tmp.data() );
    job->display( false );
    connect( job, SIGNAL( finished( HTMLCacheJob * ) ), this, SLOT( slotJobFinished( HTMLCacheJob * ) ) );
    connect( job, SIGNAL( error( HTMLCacheJob*, int, const char* ) ),
	     this, SLOT( slotError( HTMLCacheJob*, int, const char* ) ) );
    staticJobList.append( job );
    instanceJobList.append( job );
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
    for ( j = staticJobList.first(); j != 0L; j = staticJobList.next() )
	if ( strcmp( j->getSrcURL(), _url ) == 0 )
	{
	    j->cancel();
	    staticJobList.removeRef( j );
	    return;
	}
}

void HTMLCache::slotError( HTMLCacheJob *_job, int, const char * )
{
    disconnect( _job, 0, this, 0 );
    slotJobFinished( _job );
}

void HTMLCache::slotJobFinished( HTMLCacheJob* _job )
{
    urlDict.insert( _job->getSrcURL(), new QString( _job->getDestURL() + 5 ) );

    // Tell all instances
    HTMLCache *p;
    for ( p = instanceList.first(); p != 0L; p = instanceList.next() )
	p->urlLoaded( _job );

    // Remove the job from the list
    staticJobList.removeRef( _job );
    instanceJobList.removeRef( _job );
    
    // Is there another URL waiting for a job ?
    if ( !todoURLList.isEmpty() )
    {
	// Get next URL
	QString tmp = todoURLList.first();
	// Delete it from the list
	todoURLList.removeRef( todoURLList.first() );
	// Rquest this URL now
	slotURLRequest( tmp );
    }
}

void HTMLCache::urlLoaded( HTMLCacheJob *_job )
{    
    // Are we waiting for this URL ?
    if ( waitingURLList.find( _job->getSrcURL() ) != - 1 )
    {
	emit urlLoaded( _job->getSrcURL(), _job->getDestURL() + 5 );
	// We dont to wait for it now any more
	waitingURLList.remove( _job->getSrcURL() );
    }
}

bool HTMLCache::waitsForURL( const char *_url )
{
    if ( waitingURLList.find( _url ) == -1 )
	return false;
    
    return true;
}

void HTMLCache::slotCheckinURL( const char* _url, const char *_data )
{
    QString tmp;
    tmp.sprintf( "%s%i.%i", cachePath.data(), time( 0L ), fileId++ );
    FILE *f = fopen( tmp.data() + 5, "wb" );
    if ( f == 0 )
    {
	warning( "Could not write to cache\n");
	return;
    }
    
    fwrite( _data, 1, strlen( _data ), f );
    fclose( f );
    urlDict.insert( _url, new QString( tmp.data() + 5 ) );
}

const char* HTMLCache::isCached( const char *_url )
{
    QString *s = urlDict[ _url ];
    if ( s != 0L )
	return s->data();
    
    return 0L;
}

HTMLCache::~HTMLCache()
{
    instanceList.removeRef( this );
}

void HTMLCache::quit()
{
    HTMLCacheJob *job;
    for ( job = staticJobList.first(); job != 0L; job = staticJobList.next() )
	job->cancel();
    staticJobList.clear();
}

void HTMLCache::stop()
{
    HTMLCacheJob *job;
    for ( job = instanceJobList.first(); job != 0L; job = instanceJobList.next() )
    {
	staticJobList.removeRef( job );	
	job->cancel();
    }
    
    instanceJobList.clear();
}

#include "htmlcache.moc"

