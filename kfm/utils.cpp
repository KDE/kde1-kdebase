#include "utils.h"

void openWithOldApplication( const char *_cmd, QStrList& _urlList )
{
    // Find out wether there are some URL with a
    // protocol != "file"
    bool prot = FALSE;
    char *s;
    for ( s = _urlList.first(); s != 0L; s = _urlList.next() )
    {
      KURL u( s );
      if ( !u.isMalformed() )
	if ( strcmp( u.protocol(), "file" ) != 0 )
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
      
      KMimeBind::runCmd( "kfmexec", list );
    }
    // Only local files
    else
    {	
      QString tmp = "";
      
      char *s;
      for ( s = _urlList.first(); s != 0L; s = _urlList.next() )
      {
	tmp += "\"";
	KURL file = s;    
	
	QString decoded( file.path() );
	KURL::decodeURL( decoded );
	decoded = KIOServer::shellQuote( decoded ).data();
	tmp += decoded.data();
	tmp += "\" ";
      }
      
      int pos;
      while ( ( pos = cmd.find( "%f" )) != -1 )
	cmd.replace( pos, 2, tmp );

      KMimeBind::runCmd( cmd.data() );
    }
}
