

#include <pwd.h>
#include <qapp.h>
#include <X11/Xlib.h>
#include <qbitmap.h>
#include <kapp.h>
#include <qpushbt.h>
#include <qlined.h>
#include <qframe.h>
#include <qkeycode.h>
#include "saver.h"

#include "saver.moc"

int checkPasswd(char *);

kScreenSaver::kScreenSaver( Drawable drawable ) : QObject()
{
	Window root;
	int ai;
	unsigned int au;

	d = drawable;
	gc = XCreateGC( qt_xdisplay(), d, 0, 0);
	XGetGeometry( qt_xdisplay(), drawable, &root, &ai, &ai,
		&width, &height, &au, &au ); 
}

kScreenSaver::~kScreenSaver()
{
	XFreeGC( qt_xdisplay(), gc );
}

//-----------------------------------------------------------------------------

KPasswordDlg::KPasswordDlg( QWidget *parent ) : QWidget( parent )
{
	setCursor( arrowCursor );

	password = "";

	QFrame *frame = new QFrame( this );
	frame->setFrameStyle( QFrame::Panel | QFrame::Raised );
	frame->setLineWidth( 2 );
	frame->setGeometry( 0, 0, 200, 100 );

	QFont font( "helvetica", 18 );
	label = new QLabel( "Enter Password", frame );
	label->setGeometry( 20, 20, 160, 30 );
	label->setAlignment( AlignCenter );
	label->setFont( font );

	resize( 200, 100 );

	connect( &timer, SIGNAL( timeout() ), SLOT( timeout() ) );

	timerMode = 0;
	timer.start( 10000, TRUE );
}

void KPasswordDlg::keyPressed( QKeyEvent *e )
{
	switch ( e->key() )
	{
		case Key_Backspace:
			{
				int len = password.length();
				if ( len )
					password.truncate( len - 1 );
			}
			break;

		case Key_Return:
			if ( tryPassword() )
				emit passOk();
			else
			{
				label->setText( "Failed" );
				password = "";
				timerMode = 1;
				timer.start( 1500, TRUE );
			}
			break;

		case Key_Escape:
			emit passCancel();
			break;

		default:
			if ( password.length() < 50 )
			{
				password += (char)e->ascii();
				timer.changeInterval( 10000 );
			}
	}
}

int KPasswordDlg::tryPassword()
{
/*
	FILE *fp;
	passwd *pass;
	char salt[3];

#ifdef __FreeBSD__
      pass = getpwuid(getuid());
 
       if ( !strcmp( crypt( password, pass->pw_passwd), pass->pw_passwd )) {
         return(TRUE);
       } else {
         return(FALSE);
       }
#else
	if ( ( fp = fopen( "/etc/passwd", "r" ) ) != NULL )
	{
		do
		{
			pass = fgetpwent( fp );
		}
		while ( pass && pass->pw_uid != getuid() );

		fclose( fp );
		
		if ( pass )
		{
			// check if you have a shadow password system
			// 1. does /etc/shadow exist and
			fp = fopen( "/etc/shadow", "r" );
			// 2. is password entry in /etc/passwd == "x"

			if ( ( strcmp( pass->pw_passwd, "x" ) == 0 ) && ( fp != NULL ) )
			{
				// shadow password system
				char *u_name = new char[ strlen(pass->pw_name)+1 ];
				strcpy( u_name, pass->pw_name );

				do
				{
					pass = fgetpwent( fp );
				}
				while ( pass && strcmp( pass->pw_name, u_name ) != 0 );

				delete [] u_name;
			}

			if ( fp )
				fclose( fp );

			salt[0] = pass->pw_passwd[0];
			salt[1] = pass->pw_passwd[1];
			salt[2] = '\0';

			if ( !strcmp( crypt( password, salt ), pass->pw_passwd ) )
			{
				return TRUE;
			}
		}
	}

	return FALSE;
#endif
*/
  int e = checkPasswd(password.data());
  return e;
}

void KPasswordDlg::timeout()
{
	if ( timerMode )
	{
		label->setText( "Enter Password" );
		timerMode = 0;
		timer.start( 5000, TRUE );
	}
	else
		emit passCancel();
}

