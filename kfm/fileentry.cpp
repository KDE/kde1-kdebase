
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "fileentry.h"
#include <qapp.h>
#include <qkeycode.h>

#include <sys/types.h>
#include <dirent.h>      
#include <unistd.h>
#include <sys/stat.h>

KFileEntry::KFileEntry( QWidget *_parent, const char *_dir ) : QLineEdit( _parent )
{
    directory = _dir;
    directory.detach();
    possibility = -1;
}

KFileEntry::~KFileEntry()
{
}

void KFileEntry::keyPressEvent( QKeyEvent *_ev )
{
    if ( ( _ev->key() != Key_D && _ev->key() != Key_S ) || _ev->state() != ControlButton )
    {
	possibility = -1;
	QLineEdit::keyPressEvent( _ev );
	return;
    }

    bool ctrld = false;
    if (_ev->key() == Key_D && _ev->state() == ControlButton )
	ctrld = true;
    
    QString txt = text();
    if ( txt.left(6) != "file:/" && txt.find( ":/" ) != -1 )
	return;
    
    int offset = 0;
    if ( txt.left(6) == "file:/" )
	offset = 5;

    // Get the currently entered full qualified path
    QString dir = txt.data() + offset;
    
    if ( dir.data()[ 0 ] != '/' )
    {
	dir = directory.data();
	if ( dir.right(1) != "/" )
	    dir += "/";
	dir += txt.data() + offset;
    }
    
    // Get the last directory
    if ( !ctrld || possibility == -1 )
    {
	int pos = dir.findRev( "/" );
	dir2 = dir.left( pos );
	if ( dir2 == "" )
	    dir2 = "/";
	guess = dir.right( dir.length() - pos - 1 );
    }
    
    DIR *dp;
    struct dirent *ep;
    dp = opendir( dir2 );
    if ( dp == NULL )
    {
	QApplication::beep();
	return;
    }

    const char* matched = 0L;
    int len = guess.length();
    bool multiple = false;
    QString max;
    
    QStrList strlist( true );    
    if ( ctrld && possibility == -1 )
	possibilityList.clear();
    
    // Loop thru all directory entries
    while ( ( ep = readdir( dp ) ) != 0L && ( !ctrld || possibility == -1 ) )
    {
	if ( strcmp( ep->d_name, "." ) != 0L && strcmp( ep->d_name, ".." ) != 0L )
	{
	    strlist.inSort( ep->d_name );
	    if ( strncmp( ep->d_name, guess.data(), len ) == 0L )
	    {
		if ( ctrld )
		{
		    possibilityList.inSort( ep->d_name );		
		}
		// More than one possibility ?
		if ( matched != 0L )
		{
		    // Find maximum overlapping
		    int i = 0;
		    while ( matched[i] != 0 && ep->d_name[i] != 0 && matched[i] == ep->d_name[i] ) i++;
		    max = matched;
		    max = max.left(i).data();
		    matched = max.data();
		    multiple = true;
		    QApplication::beep();
		}
		else
		    matched = strlist.current();
	    }
	}
    }
    
    (void) closedir( dp );


    if ( matched == 0L && !ctrld )
    {
	QApplication::beep();
	return;
    }

    
    if ( ctrld )
    {
	if ( possibilityList.count() == 0 )
	{
	    QApplication::beep();
	    return;
	}
	
	if ( possibility >= (int)possibilityList.count() )
	    possibility = 0;
	if ( possibility == -1 )
	    possibility = 0;
	
	QString result = dir2.data();
	if ( result.right(1) != "/" )
	    result += "/";
	result += possibilityList.at( possibility++ );

	struct stat buff;
	stat( result.data(), &buff );
	if ( S_ISDIR( buff.st_mode ) )
	    result += "/";
	
	setText( result );
    }
    else
    {
	QString result = dir2.data();
	if ( result.right(1) != "/" )
	    result += "/";
	result += matched;
	
	if ( multiple )
	    QApplication::beep();
	else
	{
	    struct stat buff;
	    stat( result.data(), &buff );
	    if ( S_ISDIR( buff.st_mode ) )
		result += "/";
	}
	
	setText( result );
    }
    
    _ev->accept();

    QKeyEvent ev( Event_KeyPress, Key_End, 0, 0 );
    QLineEdit::keyPressEvent( &ev );
}

#include "fileentry.moc"

