

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
extern KLocale *glocale;

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
	label = new QLabel( glocale->translate("Enter Password"), frame );
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
				label->setText( glocale->translate("Failed") );
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
    int e = checkPasswd(password.data());
    return e;
}

void KPasswordDlg::timeout()
{
    if ( timerMode )
	{
	    label->setText( glocale->translate("Enter Password") );
	    timerMode = 0;
	    timer.start( 5000, TRUE );
	}
    else
	emit passCancel();
}

