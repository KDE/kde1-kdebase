#include <qdir.h>
#include "utils.h"
#include <stdlib.h>
#include <sys/stat.h>
#include <kfm.h>

#include <kurl.h>

void openWithOldApplication( const char *_cmd, QStrList& _urlList, const char *_workdir )
{
    // Find out wether there are some URL with a
    // protocol != "file"
    // printf("openWithOldApplication : _workdir=%s\n",_workdir);
    bool prot = FALSE;
    char *s;
    for ( s = _urlList.first(); s != 0L; s = _urlList.next() )
    {
	KURL u( s );
	if ( !u.isMalformed() )
	    if ( strcmp( u.protocol(), "file" ) != 0 || u.hasSubProtocol() )
		prot = TRUE;
    }
    
    // Where should we place the files ?
    QString cmd ( _cmd );
    // Did we get information about that ?
    if ( cmd.find( "%f" ) == -1 )
	cmd += " %f";	
    
    // Are there externel files ?
    if ( prot )
    {
	QStrList list;
	list.append( cmd );
	
	for ( s = _urlList.first(); s != 0L; s = _urlList.next() )
	{
	    list.append( s );
	}
	
	KMimeBind::runCmd( "kfmexec", list, _workdir );
    }
    // Only local files
    else
    {	
	QString tmp = "";
	
	char *s;
	for ( s = _urlList.first(); s != 0L; s = _urlList.next() )
	{
	    tmp += "\"";
	    KURL file( s );
	    tmp += KIOServer::shellQuote( file.path() );
	    tmp += "\" ";
	}
	
	int pos;
	while ( ( pos = cmd.find( "%f" )) != -1 )
	    cmd.replace( pos, 2, tmp );
	
	// printf("Executing '%s'\n",cmd.data());

	KMimeBind::runCmd( cmd.data(), _workdir );
    }
}

int testNestedURLs( const char *_src, const char *_dest )
{
  // printf("int testNestedURLs( _src=%s, _dest=%s )\n",_src,_dest);
  
    // The quick check
    if ( strcmp( _src, _dest ) == 0 )
	return 2;

    KURL u1( _src );
    KURL u2( _dest );
    if ( u1.isMalformed() || u2.isMalformed() )
	return -1;
    if ( !u1.isLocalFile() || !u2.isLocalFile() )
    {
	// Inclusion ?
	if ( strncmp( _src, _dest, strlen( _src ) ) == 0 )
	    return 1;

	return 0;
    }
    
    QString canonical1;
    QString canonical2;

    // Get the canonical path.
    QDir dir( u1.path() );
    canonical1 = dir.canonicalPath();
    if ( canonical1.isEmpty() )
	canonical1 = u1.path();

    QDir dir2( u2.path() );
    canonical2 = dir2.canonicalPath();
    if ( canonical2.isEmpty() )
	canonical2 = u2.path();
	
    int i = 0;
    
    struct stat buff;
    if ( stat( canonical1, &buff ) == 0 && S_ISDIR( buff.st_mode ) )
	i++;
    if ( stat( canonical2, &buff ) == 0 && S_ISDIR( buff.st_mode ) )
	i++;
    
    // both files
    if ( i == 0 )
    {
      // printf("BOTH files\n");
      return ( strcmp( canonical1, canonical2 ) == 0L ? 2 : 0 );
    }
    
    // One directory and one file ?
    if ( i == 1 )
    {
      // printf("One File\n");
      return 0;
    }
    
    // printf("Two directories\n");
    
    // Both directories
    if ( canonical1.right(1) != "/" )
	canonical1 += "/";
    if ( canonical2.right(1) != "/" )
	canonical2 += "/";
    
    // Are both symlinks equal ?
    if ( strcmp( canonical1, canonical2 ) == 0 )
	return 2;
    if ( strncmp( canonical1, canonical2, canonical1.length() ) == 0 )
	return 1;

    return 0;
}

QString stringSqueeze( const char *str, unsigned int maxlen )
{
    QString s ( str );
    if (s.length() > maxlen) {
        int part = (maxlen-3)/2;
        return QString(s.left(part) + "..." + s.right(part));
    }
    else return s;
}

void encodeFileName( QString& fn )
{
  int i = 0;
  while( ( i = fn.find( "/", i ) ) != -1 )
  {
    fn.replace( i, 1, "%2F" );
  }
}

void decodeFileName( QString& fn )
{
  int i = 0;
  while( ( i = fn.find( "%2F", i ) ) != -1 )
  {
    fn.replace( i, 3, "/" );
  }

  i = 0;
  while( ( i = fn.find( "%2f", i ) ) != -1 )
  {
    fn.replace( i, 3, "/" );
  }
}
