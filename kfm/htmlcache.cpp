#include <time.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#include "htmlcache.h"
#include "kbind.h"
#include "utils.h"

#include "config-kfm.h"
#include "kfmpaths.h"

#include <qfileinf.h>
#include <qdatetm.h>
#include <qdir.h>
#include <qmsgbox.h>
#include <kstring.h>

QList<HTMLCacheJob> *HTMLCache::staticJobList;
QDict<QString> *HTMLCache::urlDict;
QList<HTMLCache> *HTMLCache::instanceList;
int HTMLCache::fileId = 0;
bool HTMLCache::bCacheEnabled = true;
bool HTMLCache::bSaveCacheEnabled = true;

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
    //debug("%p HTMLCacheJob::slotJobFinished begin",this);
    emit finished( this );
    //debug("%p HTMLCacheJob::slotJobFinished end",this);
}

void HTMLCacheJob::slotError( int _kioerror, const char* _text )
{
    emit error( this, _kioerror, _text );
}

HTMLCache::HTMLCache()
{
    instanceList->append( this );
    
    staticJobList->setAutoDelete( false );
    urlDict->setAutoDelete( true );
    todoURLList.setAutoDelete( TRUE );
    instanceJobList.setAutoDelete( false );
}

void HTMLCache::slotURLRequest( const char *_url )
{
    // Is the URL already cached ?
    QString *s = 0L;
    if ( bCacheEnabled ) 
	s = (*urlDict)[ _url ];
    if ( s != 0L )
    {
	emit urlLoaded(  _url, s->data() );
	return;
    }
    
    // Are we waiting for this URL already ?
    // We can see all jobs of every instance here.
    HTMLCacheJob *job;
    for ( job = staticJobList->first(); job != 0L; job = staticJobList->next() )
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
    
    KURL u( _url );
    
    QString tmp( "file:" );
    tmp << KFMPaths::CachePath().data() << "/" << time(0L) << "." << fileId++ << u.filename();
    
    job = new HTMLCacheJob( _url, tmp.data() );
    job->display( false );
    connect( job, SIGNAL( finished( HTMLCacheJob * ) ), this, SLOT( slotJobFinished( HTMLCacheJob * ) ) );
    connect( job, SIGNAL( error( HTMLCacheJob*, int, const char* ) ),
	     this, SLOT( slotError( HTMLCacheJob*, int, const char* ) ) );
    staticJobList->append( job );
    instanceJobList.append( job );
    //debug("%p added to instanceJobList",job);
    job->copy();
}

void HTMLCache::slotCancelURLRequest( const char *_url )
{
    waitingURLList.remove( _url );

    // Does anyone else wait for it ?
    HTMLCache *p;
    for ( p = instanceList->first(); p != 0L; p = instanceList->next() )
	if ( p->waitsForURL( _url ) )
	     return;

    // Find the job that works on this and kill him
    HTMLCacheJob *j;
    for ( j = staticJobList->first(); j != 0L; j = staticJobList->next() )
	if ( strcmp( j->getSrcURL(), _url ) == 0 )
	{
	    j->cancel();
	    staticJobList->removeRef( j );
	    return;
	}
}

void HTMLCache::slotError( HTMLCacheJob *_job, int, const char * )
{
    //debug("%p HTMLCache::slotError",_job);
    disconnect( _job, 0, this, 0 );
    slotJobFinished( _job );
}

void HTMLCache::slotJobFinished( HTMLCacheJob* _job )
{
    //debug("%p HTMLCache::slotJobFinished",_job);

    // Remove "file:" but not using KURL, it will change /tmp//... into /tmp/...
    QString * s = new QString ( _job->getDestURL() + 5 );
    // Test if file exists really (job might have been cancelled)
    // and is not empty (happens often when the window is closed while loading)
    struct stat buff; 
    if ( lstat( s->data(), &buff ) == 0 && buff.st_size > 0)
    {
        //debug("HTMLCACHE::slotJobFinished : inserting %s",s->data());
        urlDict->insert( _job->getSrcURL(), s );
    }

    // Tell all instances
    HTMLCache *p;
    for ( p = instanceList->first(); p != 0L; p = instanceList->next() )
	p->urlLoaded( _job );

    // Remove the job from the list
    staticJobList->removeRef( _job );
    instanceJobList.removeRef( _job );
    //debug("%p removed from instanceJobList",_job);

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
    KURL u( _job->getDestURL() );
    emit urlLoaded( _job->getSrcURL(), u.path() );
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
    tmp.sprintf( "%s/%i.%i", KFMPaths::CachePath().data(), time( 0L ), fileId++ );
    FILE *f = fopen( tmp.data(), "wb" );
    if ( f == 0 )
    {
	warning( "Could not write to cache\n");
	return;
    }
    
    fwrite( _data, 1, strlen( _data ), f );
    fclose( f );
    //debug("HTMLCACHE::slotCheckinURL : inserting %s",tmp.data());
    urlDict->insert( _url, new QString( tmp.data() ) );
}

const char* HTMLCache::isCached( const char *_url )
{
    QString *s = (*urlDict)[ _url ];
    if ( s != 0L )
	return s->data();
    
    return 0L;
}

HTMLCache::~HTMLCache()
{
    //debug("HTMLCache::~HTMLCache()");
    // stop(); // called by kfmview destructor before "delete htmlCache".
    instanceList->removeRef( this );
}

void HTMLCache::quit()
{
    HTMLCacheJob *job;
    for ( job = staticJobList->first(); job != 0L; job = staticJobList->next() )
	job->cancel();
    staticJobList->clear();
}

void HTMLCache::stop()
{
    //debug("HTMLCache::stop() : instanceJobList.count()=%d",instanceJobList.count());
    HTMLCacheJob *job;
    for ( job = instanceJobList.first(); job != 0L; job = instanceJobList.next() )
    {
	staticJobList->removeRef( job );	
        disconnect(job, 0, this, 0); // we don't want slotJobFinished to be called !
	job->cancel();
        //debug("%p, from instanceJobList, cancelled",job);
    }
    
    instanceJobList.clear();
    //debug("instanceJobList empty");
}

void HTMLCache::save()
{
    if (! bSaveCacheEnabled) return;
    QString p = KFMPaths::CachePath().data();
    p += "/index.html";
    FILE *f = fopen( p, "w" );
    if ( f == 0L )
    {
	warning("Could not write '%s'\n", p.data() );
	return;
    }
    
    fprintf( f, "<HTML><HEAD><TITLE>KFM Cache</TITLE></HEAD><BODY><H2>Contents of the cache</H2>\n" );
    fprintf( f, "<p>Use the icons for drag and drop actions, since they refere to the cache files " );
    fprintf( f, "on your hard disk, while you should use the textual links for browsing</p><hr>\n" );
    fprintf( f, "<table>\n" );
    
    QDictIterator<QString> it( *urlDict );
    for ( ; it.current(); ++it )
	// Do not save the VERY large URL's. Some people store complete stories
	// in the URL's reference part!
	if ( it.current()->length() < 1024 )
	{
	    fprintf( f, "<tr><td><a href=\"%s\"><img border=0 src=\"%s\"></a></td> ",
		     it.current()->data(), KMimeType::getPixmapFileStatic( it.current()->data(), TRUE ) );
	    fprintf( f, "<td><a href=\"%s\">\n%s\n</a></td>", it.currentKey(), it.currentKey() );
	    QFileInfo finfo( it.current()->data() );
	    fprintf( f, "<td>%s</td></tr>\n", finfo.lastModified().toString().data() );
	}
    
    fprintf( f, "</table></BODY></HTML>\n" );
    fclose( f );

    p = KFMPaths::CachePath().data();
    p += "/index.txt";
    f = fopen( p, "w" );
    if ( f == 0L )
    {
      warning("Could not write '%s'\n", p.data() );
      return;
    }

    QDictIterator<QString> it2( *urlDict );
    for ( ; it2.current(); ++it2 )
      fprintf( f, "%s\n%s\n" , it2.currentKey(), it2.current()->data() );
    
    fclose( f );
}

void HTMLCache::load()
{
  QString path = KFMPaths::CachePath().data();
  path += "/index.txt";
  
  FILE *f = fopen( path, "r" );
  if ( f != 0L )
  {
    QString url;
    char buffer[ 2048 ];
    char *p = 0L;
    do
    {
      p = fgets( buffer, 2048, f );
      if ( p )
      {
	url = buffer;
	if ( url.right(1) == "\n" )
	  url.truncate( url.length() - 1 );
	p = fgets( buffer, 2048, f );
	if ( p )
        {
	  QString *s = new QString( buffer );
	  if ( s->right(1) == "\n" )
	    s->truncate( s->length() - 1 );
	  
	  // Does file really exist ?
	  struct stat buff;
	  if ( lstat( s->data(), &buff ) == 0 ) {
            //debug("HTMLCACHE : inserting %s",s->data());
	    urlDict->insert( url, s );
          } //else debug("HTMLCACHE : _not_ inserting %s",s->data());
	}
      }
    } while ( p );

    fclose( f );
  }
  else
  {
    warning("Could not read '%s'\n", path.data() );
  }

  // Delete files which are not in the dict
  // except index.html and index.txt
  DIR *dp = 0L;
  struct dirent *ep;
  
  dp = opendir( KFMPaths::CachePath() );
  if ( dp == 0L )
  {
    warning("Could not scan %s", KFMPaths::CachePath().data() );
    return;
  }

  QFileInfo finfo(KFMPaths::CachePath().data());
  if (finfo.isSymLink() || !finfo.isDir()) {
      QMessageBox::warning( 0L, i18n("KFM Error"), KFMPaths::CachePath()+i18n(
          " is not a directory ! Security Alert !\nPlease check it.\n"));
      return;
  }
  
  while ( ( ep = readdir( dp ) ) != 0L )
  {
    if ( strcmp( ep->d_name, "." ) != 0L && strcmp( ep->d_name, ".." ) != 0L
         && strcmp( ep->d_name, "index.txt" ) != 0L && strcmp( ep->d_name, "index.html" ) != 0L )
    {
      QString name( KFMPaths::CachePath().data() );
      name += "/";
      name += ep->d_name;
      
      bool found = false;
      QDictIterator<QString> it2( *urlDict );
      for ( ; !found && it2.current(); ++it2 )
	if ( strcmp( it2.current()->data(), name.data() ) == 0 )
	  found = true;

      if ( !found )
      {
        //debug("HTMLCACHE : deleting %s",name.data());
	unlink( name.data() );
      }
    }
  }
}

void HTMLCache::clear()
{
    QFileInfo finfo(KFMPaths::CachePath().data());
    if (finfo.isSymLink() || !finfo.isDir()) {
      QMessageBox::warning( 0L, i18n("KFM Error"), KFMPaths::CachePath()+i18n(
         " is not a directory ! Security Alert !\nCheck it before clearing the cache.\n"));
      return;
    }
    QStrList todie;
    
    QDictIterator<QString> it( *urlDict );
    for ( ; it.current(); ++it ) {
	if ( unlink( it.current()->data() ) == 0 )
	    todie.append( it.currentKey() );
    }

    const char *s;
    for ( s = todie.first(); s != 0L; s = todie.next() )
	urlDict->remove( s );
    
    save();
}

void HTMLCache::enableCache( bool _enable )
{
    bCacheEnabled = _enable;
}

bool HTMLCache::isEnabled(){
    return bCacheEnabled;
}

void HTMLCache::enableSaveCache( bool _enable )
{
    bSaveCacheEnabled = _enable;
}

bool HTMLCache::isSaveEnabled(){
    return bSaveCacheEnabled;
}
#include "htmlcache.moc"

