#include <qstring.h>
#include <qmsgbox.h>

#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

#include "kbind.h"
#include "kfmwin.h"

// Executes all apps/documents in the $HOME/Desktop/Autostart directory

void autostart()
{
    QString url = "file:";
    url += getenv( "HOME" );
    url += "/Desktop/Autostart/";
    
    DIR *dp;
    struct dirent *ep;
    KURL u1( url );
    dp = opendir( u1.path() );
    if ( dp == NULL )
    {
	QMessageBox::message( "KFM Installation Error", "The directory " + url +
			      " does not exist\nRun 'setupKFM' to solve the problem" );
	exit(1);
    }
    
    // Loop thru all directory entries
    while ( ep = readdir( dp ) )
    {
	if ( strcmp( ep->d_name, "." ) != 0 && strcmp( ep->d_name, ".." ) != 0 )
	{
	    QString u2 = url.data();
	    u2 += ep->d_name;
	    
	    KURL u( u2 );
	    // Is it a directory? If not ...
	    if ( !KIOServer::isDir( u2 ) )
	    {
		// Did the user double click on a *.kdelnk file ?
		QString tmp = u2;
		if ( tmp.length() > 7 && tmp.right(7) == ".kdelnk" )
		{
		    KFileType::runBinding( u2 );
		}
		// Are we entering a tar file ?
		else if ( strstr( u2, ".tar" ) != 0 || strstr( u2, ".tgz") != 0 || strstr( u2, ".tar.gz" ) != 0 )
		{	
		    QString t( "tar:" );
		    t += u.path();
		    
		    KFileWindow *m = new KFileWindow( 0L, 0L, t.data() );
		    m->show();
		}
		else
		{
		    struct stat buff;
		    if ( 0 > stat( u.path(), &buff ) )
		    {
			printf("ERROR: Could not access file %s\n",u2.data() );
		    }
		    else
		    {
			
			// Executable ?
			if ( ( buff.st_mode & ( S_IXUSR | S_IXGRP | S_IXOTH ) != 0 ) )
			{
			    char buffer[ 1024 ];
			    sprintf( buffer, "%s &",u.path() );
			    system( buffer );
			}
			// Start the appropriate binding.
			else
			{
			    KFileType::runBinding( u2 );
			}   
		    }
		}
	    }
	    else
	    {
		KFileWindow *m = new KFileWindow( 0L, 0L, u2 );
		m->show();
	    }
	}
    }
    (void) closedir( dp );
}
