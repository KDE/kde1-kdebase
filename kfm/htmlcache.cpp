#include <time.h>

#include "htmlcache.h"

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
    connect( this, SIGNAL( finished( int ) ), this, SLOT( slotJobFinished( int ) ) );
}

void HTMLCacheJob::copy()
{
    KIOJob::copy( url.data(), dest.data() );
}

HTMLCacheJob::~HTMLCacheJob()
{
}

void HTMLCacheJob::slotJobFinished( int _id )
{
    emit finished( this );
}

HTMLCache::HTMLCache()
{
    instanceList.append( this );
    
    char *p = getenv( "HOME" );
    if ( !p )
    {
	printf("ERROR: $HOME is not defined\n");
	exit(1);
    }    
    cachePath.sprintf( "file:%s/.kfm/cache/", p );

    htmlJobList.setAutoDelete( FALSE );
    urlDict.setAutoDelete( TRUE );
}

void HTMLCache::slotURLRequest( const char *_url )
{
    QString *s = urlDict[ _url ];
    if ( s != 0L )
    {
	printf("&&&&&&&&&&&&&& Having '%s' in cache as '%s'\n",_url,s->data());
	emit urlLoaded(  _url, s->data() );
	return;
    }

    printf("&&&&&&&&&&&&&& Waiting for '%s'\n",_url);

    if ( waitingURLList.find( _url ) == -1 )
	waitingURLList.append( _url );
    
    // Are we waiting for this URL already ?
    HTMLCacheJob *job;
    for ( job = htmlJobList.first(); job != 0L; job = htmlJobList.next() )
	if ( strcmp( _url, job->getSrcURL() ) == 0 )
	    return;
    
    QString tmp;
    tmp.sprintf( "%s%i.%i", cachePath.data(), time( 0L ), fileId++ );
    printf("&&&&&&&&&&&&&&&&&& New job for '%s' to '%s'\n",_url,tmp.data());
    job = new HTMLCacheJob( _url, tmp.data() );
    job->display( FALSE );
    connect( job, SIGNAL( finished( HTMLCacheJob * ) ), this, SLOT( slotJobFinished( HTMLCacheJob * ) ) );
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
    printf("&&&&&&&&&&&&&&&&&& URL '%s' loaded in '%s'\n",_job->getSrcURL(), _job->getDestURL() + 5 );
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
	urlDict.insert( _job->getSrcURL(), new QString( _job->getDestURL() + 5 ) );
	emit urlLoaded( _job->getSrcURL(), _job->getDestURL() + 5 );
	// We dont to wait for it now any more
	waitingURLList.remove( _job->getSrcURL() );
    }
}

bool HTMLCache::waitsForURL( const char *_url )
{
    if ( waitingURLList.find( _url ) == -1 )
	return FALSE;
    
    return TRUE;
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
