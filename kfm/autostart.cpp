#include <qstring.h>
#include <qmsgbox.h>

#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

#include "kbind.h"
#include "kfmgui.h"
#include "kfmpaths.h"
#include "kfmexec.h"
#include "config-kfm.h"

#include <klocale.h>

// Executes all apps/documents in the $HOME/Desktop/Autostart directory

void autostart()
{
    QString url = "file:" + KFMPaths::AutostartPath();
    
    DIR *dp;
    struct dirent *ep;

    dp = opendir( KFMPaths::AutostartPath() );
    if ( dp == NULL )
    {
	QMessageBox::critical( 0L, klocale->translate("KFM Installation Error"), 
			      klocale->translate("The directory ") + url +
			      klocale->translate(" does not exist\n") );
	exit(1);
    }
    
    // Loop thru all directory entries
    while ( ( ep = readdir( dp ) ) != 0L )
    {
	// Dont execute "..", "." and dot files like ".directory" or ".kde.html"
	if ( strcmp( ep->d_name, "." ) != 0 && strcmp( ep->d_name, ".." ) != 0 &&
	     ep->d_name[0] != '.' && strcmp( ep->d_name, "index.html" ) != 0 )
	{
	    QString u2 (ep->d_name);
	    KURL::encodeURL(u2); // in case of '/' in the name (#619)
	    u2 = url + u2;
	    
	    KFMExec *e = new KFMExec();
	    e->openURL( u2 );
	} 
    }
    (void) closedir( dp );
}
