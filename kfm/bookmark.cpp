#include <qdir.h>

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stddef.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

#include <ksimpleconfig.h>
#include <kurl.h>
#include <kapp.h>
#include <kwm.h>
#include <qmsgbox.h>

#include "bookmark.h"
#include "kbind.h"

/**
 * Gloabl ID for bookmarks.
 */
int g_id = 0;

/********************************************************************
 *
 * KBookmarkManager
 *
 ********************************************************************/

KBookmarkManager::KBookmarkManager() : m_Root( 0L, 0L, 0L )
{
  // Little hack
  m_Root.m_pManager = this;
  m_bAllowSignalChanged = true;
  m_bNotify = true;
  
  QString p = getenv( "HOME" );
  p += "/.kde/share/apps/kfm/bookmarks";
  scan( p );

  connect( KIOServer::getKIOServer(), SIGNAL( notify( const char* ) ),
	   this, SLOT( slotNotify( const char* ) ) );
}

void KBookmarkManager::slotNotify( const char *_url )
{
  if ( !m_bNotify )
    return;
  
  KURL u( _url );
  if ( strcmp( u.protocol(), "file" ) != 0 )
    return;
  
  QString p = getenv( "HOME" );
  p += "/.kde/share/apps/kfm/bookmarks";
  QDir dir2( p );
  QDir dir1( u.path() );

  QString p1( dir1.canonicalPath() );
  QString p2( dir2.canonicalPath() );
  if ( p1.isEmpty() )
    p1 = u.path();
  if ( p2.isEmpty() )
    p2 = p.data();
  
  printf("Checking '%s' and '%s'\n",p1.data(),p2.data() );
  
  if ( strncmp( p1.data(), p2.data(), p2.length() ) == 0 )
  {
    QString d = getenv( "HOME" );
    d += "/.kde/share/apps/kfm/bookmarks/";
    scan( d );
  }
}
  
void KBookmarkManager::emitChanged()
{
  // Scanning right now ?
  if ( m_bAllowSignalChanged )
    {
      // ... no => emit signal
      emit changed();
      //tell krootwm to refresh the bookmarks popup menu
      KWM::sendKWMCommand ("krootwm:refreshBM");
    }
}

void KBookmarkManager::scan( const char * _path )
{
  m_Root.clear();
  
  // Do not emit 'changed' signals here.
  m_bAllowSignalChanged = false;
  scanIntern( &m_Root, _path );
  m_bAllowSignalChanged = true;
   
  emitChanged();
}

void KBookmarkManager::scanIntern( KBookmark *_bm, const char * _path )
{
  DIR *dp;
  struct dirent *ep;
  dp = opendir( _path );
  if ( dp == 0L )
    return;
  
  // Loop thru all directory entries
  while ( ( ep = readdir( dp ) ) != 0L )
  {
    if ( strcmp( ep->d_name, "." ) != 0 && strcmp( ep->d_name, ".." ) != 0 )
    {
      // QString name = ep->d_name;	

      QString file = _path;
      file += "/";
      file += ep->d_name;
      struct stat buff;
      stat( file.data(), &buff );
      if ( S_ISDIR( buff.st_mode ) )
      {
	KBookmark* bm = new KBookmark( this, _bm, KBookmark::decode( ep->d_name ) );
	scanIntern( bm, file );
      }
      else
      {    
	KSimpleConfig cfg( file, true );
	cfg.setGroup( "KDE Desktop Entry" );
	QString type = cfg.readEntry( "Type" );	
	// QString url = cfg.readEntry( "URL" );
	// QString icon = cfg.readEntry( "Icon" );
	// QString miniicon = cfg.readEntry( "MiniIcon", icon );
	// if ( !url.isEmpty() && !icon.isEmpty() && !miniicon.isEmpty() )
	if ( type == "Link" )
	{
	  new KBookmark( this, _bm, ep->d_name, cfg );
	}
      }
    }
  }
}

/********************************************************************
 *
 * KBookmark
 *
 ********************************************************************/

KBookmark::KBookmark( KBookmarkManager *_bm, KBookmark *_parent, const char *_text, KSimpleConfig& _cfg )
{
  ASSERT( _bm != 0L );
  ASSERT( _parent != 0L );
  
  _cfg.setGroup( "KDE Desktop Entry" );
  m_url = _cfg.readEntry( "URL" );
  
  m_pPixmap = 0L;
  m_pMiniPixmap = 0L;
  m_id = g_id++;
  m_pManager = _bm;
  m_lstChildren.setAutoDelete( true );

  m_text = KBookmark::decode( _text );
  if ( m_text.length() > 7 && m_text.right( 7 ) == ".kdelnk" )
    m_text.truncate( m_text.length() - 7 );
  
  m_type = URL;
  
  m_file = _parent->file();
  m_file += "/";
  m_file += _text;

  _parent->append( this );
  
  m_pManager->emitChanged();
}

KBookmark::KBookmark( KBookmarkManager *_bm, KBookmark *_parent, const char *_text )
{
  m_pPixmap = 0L;
  m_pMiniPixmap = 0L;
  m_id = g_id++;
  m_pManager = _bm;
  m_lstChildren.setAutoDelete( true );
  m_type = Folder;
  m_text = _text;

  QString p( getenv( "HOME" ) );
  p += "/.kde/share/apps/kfm/bookmarks";
  const char *dir = p;
  if ( _parent )
    dir = _parent->file();
  m_file = dir;
  if ( _text )
  {
    m_file += "/";
    m_file += encode( _text );
  }

  if ( _parent )
    _parent->append( this );
  
  if ( m_pManager )
    m_pManager->emitChanged();
}

KBookmark::KBookmark( KBookmarkManager *_bm, KBookmark *_parent, const char *_text, const char *_url )
{
  ASSERT( _bm != 0L );
  ASSERT( _parent != 0L );
  ASSERT( _text != 0L && _url != 0L );
  
  KURL u( _url );

  QString tmp;
  const char *icon = 0L;
  if ( strcmp( u.protocol(), "file" ) == 0 )
  {
    // tmp = folderType->getPixmapFile( _url );
    tmp = KMimeType::getPixmapFileStatic( _url );
    int i = tmp.findRev( '/' );
    ASSERT( i != -1 );
    icon = tmp.data() + i + 1;
  }
  else if ( strcmp( u.protocol(), "ftp" ) == 0 )
    icon = "ftp.xpm";
  else
    icon = "www.xpm";

  QString tmp2;
  const char *mini = 0L;
  if ( strcmp( u.protocol(), "file" ) == 0 )
  {
    // tmp2 = folderType->getPixmapFile( _url );
    tmp2 = KMimeType::getPixmapFileStatic( _url, true );
    int i = tmp2.findRev( '/' );
    ASSERT( i != -1 );
    mini = tmp2.data() + i + 1;
  }
  else if ( strcmp( u.protocol(), "ftp" ) == 0 )
    mini = "ftp.xpm";
  else
    mini = "www.xpm";
  
  m_pPixmap = 0L;
  m_pMiniPixmap = 0L;
  m_id = g_id++;
  m_pManager = _bm;
  m_lstChildren.setAutoDelete( true );
  m_text = _text;
  m_url = _url;
  m_type = URL;
  
  m_file = _parent->file();
  m_file += "/";
  m_file += encode( _text );
  m_file += ".kdelnk";

  FILE *f = fopen( m_file, "w" );
  if ( f == 0L )
  {
    QMessageBox::critical( (QWidget*)0L, i18n("KFM Error"), i18n("Could not write bookmark" ) );
    return;
  }
  
  fprintf( f, "# KDE Config File\n" );
  fprintf( f, "[KDE Desktop Entry]\n" );
  fprintf( f, "URL=%s\n", m_url.data() );
  fprintf( f, "Icon=%s\n", icon );
  fprintf( f, "MiniIcon=%s\n", mini );
  fprintf( f, "Type=Link\n" );
  fclose( f );

  m_pManager->disableNotify();
  
  // Update opened KFM windows. Perhaps there is one
  // that shows the bookmarks directory.
  tmp = "file:";
  QString fe( _parent->file() );
  // To make an URL, we have th encode the path
  KURL::encodeURL( fe );
  tmp += fe;
  KIOServer::sendNotify( tmp );
  
  m_pManager->enableNotify();

  _parent->append( this );
  
  m_pManager->emitChanged();
}

void KBookmark::clear()
{
  KBookmark *bm;
  
  for ( bm = children()->first(); bm != NULL; bm = children()->next() )
  {
    bm->clear();
  }
  
  m_lstChildren.clear();
}

KBookmark* KBookmark::findBookmark( int _id )
{
  if ( _id == id() )
    return this;

  KBookmark *bm;
  
  for ( bm = children()->first(); bm != NULL; bm = children()->next() )
  {
    if ( bm->id() == _id )
      return bm;
    
    if ( bm->type() == Folder )
    {
      KBookmark *b = bm->findBookmark( _id );
      if ( b )
	return b;
    }
  }

  return 0L;
}

QString KBookmark::encode( const char *_str )
{
  QString str( _str );

  int i = 0;
  while ( ( i = str.find( "%", i ) ) != -1 )
  {
    str.replace( i, 1, "%%");
    i += 2;
  }
  while ( ( i = str.find( "/" ) ) != -1 )
      str.replace( i, 1, "%2f");

  return QString( str.data() );
}

QString KBookmark::decode( const char *_str )
{
  QString str( _str );
  
  int i = 0;
  while ( ( i = str.find( "%%", i ) ) != -1 )
  {
    str.replace( i, 2, "%");
    i++;
  }
  
  while ( ( i = str.find( "%2f" ) ) != -1 )
      str.replace( i, 3, "/");
  while ( ( i = str.find( "%2F" ) ) != -1 )
      str.replace( i, 3, "/");

  return QString( str.data() );
}

QPixmap* KBookmark::pixmap()
{
  if ( m_pPixmap == 0L )
  {
    // QString f( m_file.data() );
    // KURL::encodeURL( f );
    if ( m_type == Folder )
      m_pPixmap = folderType->getPixmap( m_file, false );
    else
      m_pPixmap = kdelnkType->getPixmap( m_file, false );
  }

  ASSERT( m_pPixmap );
  
  return m_pPixmap;
}

QPixmap* KBookmark::miniPixmap()
{
  if ( m_pMiniPixmap == 0L )
  {
    // QString f( m_file.data() );
    // KURL::encodeURL( f );
    if ( m_type == Folder )
      m_pMiniPixmap = folderType->getPixmap( m_file, true );
    else
      m_pMiniPixmap = kdelnkType->getPixmap( m_file, true );
  }

  ASSERT( m_pMiniPixmap );
  
  return m_pMiniPixmap;
}


#include "bookmark.moc"
