//-----------------------------------------------------------------------------
//
// Screen savers for KDE
//
// Copyright (c)  Martin R. Jones 1996
//

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <qapp.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <qkeycode.h>
#include <qdialog.h>
#include <qpushbt.h>
#include <qbitmap.h>

#include "xautolock.h"
#include "saver.h"
#include "main.h"
#include "main.moc"
#include <kmsgbox.h>

KLocale *glocale;

extern void startScreenSaver( Drawable d );
extern void stopScreenSaver();
extern int  setupScreenSaver();
extern const char *getScreenSaverName();
extern void initPasswd();

int mode = MODE_NONE, lock = FALSE, passOk = FALSE;
bool canGetPasswd = true;
static int lockOnce = FALSE;
static int xs_timeout, xs_interval, xs_prefer_blanking, xs_allow_exposures;
static QString pidFile;
static KPasswordDlg *passDlg = NULL;
static QWidget *saverWidget = NULL;
extern char *ProgramName;
extern Bool allowroot;

void grabInput( QWidget *w );
void releaseInput();
static void lockNow( int );
static void cleanup( int );
void catchSignals();
void usage( char *name );


//----------------------------------------------------------------------------

ssApp::ssApp( int &argc, char **argv ) : KApplication( argc, argv )
{
	KConfig *kssConfig = new KConfig( kapp->kde_configdir() + "/kssrc", 
	                                  kapp->localconfigdir() + "/kssrc" );
	kssConfig->setGroup( "kss" );
	stars = kssConfig->readBoolEntry( "PasswordAsStars", true );
	delete kssConfig;
}

bool ssApp::x11EventFilter( XEvent *event )
{
	if ( passDlg )	// pass key presses to password dialog
	{
		if ( event->type == KeyPress )
		{
			XKeyEvent *xke = (XKeyEvent *) event;

			int key = 0;
			KeySym keysym = 0;
			XComposeStatus compose;
			char buffer[2] = "";
			XLookupString( xke, buffer, 1, &keysym, &compose );

			switch ( keysym )
			{
				case XK_BackSpace:
					key = Key_Backspace;
					break;

				case XK_Return:
					key = Key_Return;
					break;

				case XK_Escape:
					key = Key_Escape;
					break;
			}

			if ( buffer[0] != '\0' || key != 0 )
			{
			    QKeyEvent qke( Event_KeyPress, key, buffer[0], 0 );
			    passDlg->keyPressed( &qke );
			}
			return TRUE;
		}
		return FALSE;
	}

	if ( mode == MODE_INSTALL || mode == MODE_TEST )
	{
		if ( (event->type == KeyPress
			|| event->type == ButtonPress
			|| event->type == MotionNotify ) && !passDlg )
		{
			if ( !canGetPasswd || (!lock && !lockOnce) )
				qApp->exit_loop();
			else
			{
				passDlg = new KPasswordDlg( saverWidget, stars );
				connect(passDlg, SIGNAL(passOk()), SLOT(slotPassOk()));
				connect(passDlg, SIGNAL(passCancel()), SLOT(slotPassCancel()));
				passDlg->move( (QApplication::desktop()->width()
						- passDlg->width())/2,
					(QApplication::desktop()->height()
						- passDlg->height())/2 );
				passDlg->show();
			}
			return TRUE;
		}
		else if ( event->type == VisibilityNotify )
		{
			if ( event->xvisibility.state != VisibilityUnobscured)
			{
				if ( !passDlg )
					saverWidget->raise();
			}
		}
	}

	return FALSE;
}

void ssApp::slotPassOk()
{
	passOk = TRUE;
	delete passDlg;
	passDlg = NULL;
	qApp->exit_loop();
}

void ssApp::slotPassCancel()
{
	passOk = FALSE;
	delete passDlg;
	passDlg = NULL;
	grabInput( saverWidget );
}

//----------------------------------------------------------------------------

void grabInput( QWidget *w)
{
	XGrabKeyboard( qt_xdisplay(), QApplication::desktop()->winId(), True,
			GrabModeAsync, GrabModeAsync, CurrentTime );

	XGrabPointer( qt_xdisplay(), QApplication::desktop()->winId(), True,
			ButtonPressMask
			| ButtonReleaseMask | EnterWindowMask | LeaveWindowMask
			| PointerMotionMask | PointerMotionHintMask | Button1MotionMask
			| Button2MotionMask | Button3MotionMask | Button4MotionMask
			| Button5MotionMask | ButtonMotionMask | KeymapStateMask,
			GrabModeAsync, GrabModeAsync, None, w->cursor().handle(),
			CurrentTime );
}

void releaseInput()
{
	XUngrabKeyboard( qt_xdisplay(), CurrentTime );
	XUngrabPointer( qt_xdisplay(), CurrentTime );
}

QWidget *createSaverWindow()
{
QWidget *w;

	// WStyle_Customize sets override_redirect
	w = new QWidget( NULL, "", WStyle_Customize );

	/* set NoBackground so that the saver can capture the current
	 * screen state if neccessary
	 */
	w->setBackgroundMode( QWidget::NoBackground );

	XSetWindowAttributes attr;
	attr.event_mask = KeyPressMask | ButtonPressMask | MotionNotify |
			 VisibilityChangeMask;
	XChangeWindowAttributes(qt_xdisplay(), w->winId(), CWEventMask, &attr);

	QBitmap bm( 1, 1, TRUE );
	QCursor c( bm, bm );
	w->setCursor( c );

	grabInput( w );

	return w;
}

void destroySaverWindow( QWidget *w )
{
	releaseInput();
	delete w;
}

int main( int argc, char *argv[] )
{
	Window saveWin = 0;
	int timeout = 600;
	ProgramName = argv[0];
	ssApp a( argc, argv );

	glocale = new KLocale("klock");

	if ( argc == 1 )
	    usage( argv[0] );

	enum parameter_code { 
	    install, setup, preview, inroot, test, delay, 
	    arg_lock, corners, descr, arg_nice, help, allow_root, 
	    unknown
	} parameter;
	
	char *strings[] = { 
	    "-install", "-setup", "-preview", "-inroot", "-test", "-delay", 
	    "-lock", "-corners", "-desc", "-nice", "--help", "-allow-root", 
	    0
	};

	int i = 1;
	while ( i < argc) {
	    parameter = unknown;

	    for ( int p = 0 ; strings[p]; p++)
		if ( !strcmp( argv[i], strings[p]) ) {
		    parameter = static_cast<parameter_code>(p);
		    break;
		}

	    switch (parameter) 
		{
		case install:
		    mode = MODE_INSTALL;
		    break;
		case setup:
		    mode = MODE_SETUP;
		    break;
		case preview:
		    mode = MODE_PREVIEW;
		    saveWin = atol( argv[++i] );
		    break;
		case inroot:
		    mode = MODE_PREVIEW;
		    saveWin = kapp->desktop()->winId();
		    break;
		case test:
		    mode = MODE_TEST;
		    break;
		case delay:
		    timeout = atoi( argv[++i] ) * 60;
		    if ( timeout < 60 )
			timeout = 60;
		    break;
		case arg_lock:
		    lock = TRUE;
		    break;
		case corners:
		    setCorners( argv[++i] );
		    break;
		case descr:
		    printf( "%s\n", getScreenSaverName() );
		    exit( 0 );
		case arg_nice:
#ifdef HAVE_NICE
		    nice( atoi( argv[++i] ) );
#else
		    warning(glocale->translate(
					       "Option %s is not support on "
					       "this plattform!"), 
			    strings[arg_nice]);
#endif
		    break;
		case allow_root:
		    allowroot = 1;
		    break;
		case help:
		    usage( argv[0] );
		    break;
		default: // unknown
		    debug("unknown parameter");
		    break;
		}
	    i++;
	}

	initPasswd();
	// drop root privileges before we do anything important
	setuid(getuid());


	if ( mode == MODE_INSTALL )
	{
	    if (!canGetPasswd) {
		
		QString tmp = glocale->translate("Your system uses shadow passwords!\n"
						 "Please contact your system administrator.\n\n"
			     "Tell him, that you need suid for the screensavers!");
		
		KMsgBox::message(NULL, 
				 glocale->translate("Shadow Passwords"), 
				 tmp);
	    }
	    pidFile = getenv( "HOME" );
	    pidFile += "/.kss.pid.";
            char ksshostname[200];
            gethostname(ksshostname, 200);
            pidFile += ksshostname;

	    FILE *fp;
	    if ( (fp = fopen( pidFile, "r" ) ) != NULL )
		{
			int pid;
			fscanf( fp, "%d", &pid );
			fclose( fp );
			if ( pid != getpid() && pid > 1 )
			{
				kill( pid, SIGTERM );
				sleep( 1 );
			}
		}
		if ( (fp = fopen( pidFile, "w" ) ) != NULL )
		{
		    // on some systems, it's long one some in, so a cast may help
			fprintf( fp, "%ld\n", static_cast<long>(getpid()) );
			fclose( fp );
		}

		catchSignals();
		XGetScreenSaver( qt_xdisplay(), &xs_timeout, &xs_interval,
				&xs_prefer_blanking, &xs_allow_exposures );
		XSetScreenSaver( qt_xdisplay(), 0, xs_interval, xs_prefer_blanking,
				xs_allow_exposures );

		while ( 1 )
		{
			if ( waitTimeout( timeout ) == FORCE_LOCK )
				lockOnce = TRUE;

			saverWidget = createSaverWindow();

			saverWidget->setFixedSize( QApplication::desktop()->width(),
				 QApplication::desktop()->height() );
			saverWidget->move( 0, 0 );
//			saverWidget->setGeometry( 0, 0, QApplication::desktop()->width(),
//				 QApplication::desktop()->height() );
			saverWidget->show();
			saverWidget->raise();

			saveWin = saverWidget->winId();

			startScreenSaver( saveWin );
			a.enter_loop();
			stopScreenSaver();

			destroySaverWindow( saverWidget );

			lockOnce = FALSE;
		}
	}
	else if ( mode == MODE_TEST )
	{
		saverWidget = createSaverWindow();
		saverWidget->setFixedSize( QApplication::desktop()->width(),
				 QApplication::desktop()->height() );
		saverWidget->move( 0, 0 );
//		saverWidget->setGeometry( 0, 0, QApplication::desktop()->width(),
//				 QApplication::desktop()->height() );
		saverWidget->show();
		saverWidget->raise();

		saveWin = saverWidget->winId();

		startScreenSaver( saveWin );
		a.enter_loop();
		stopScreenSaver();

		destroySaverWindow( saverWidget );
	}
	else if ( mode == MODE_PREVIEW )
	{
		startScreenSaver( saveWin );

		a.exec();
	}
	else if ( mode == MODE_SETUP )
	{
		setupScreenSaver();
	}
	else
		usage( argv[0] );

	return 0;
}

//----------------------------------------------------------------------------

void catchSignals()
{
	// SIGUSR1 forces a lock
	signal(SIGUSR1, lockNow);

	// These signals are captured to clean-up before exiting
	signal(SIGINT, cleanup);		/* Interrupt */
	signal(SIGTERM, cleanup);		/* Terminate */

	signal(SIGABRT, cleanup);
	signal(SIGALRM, cleanup);
	signal(SIGFPE, cleanup);
	signal(SIGILL, cleanup);
	signal(SIGPIPE, cleanup);
	signal(SIGQUIT, cleanup);
	signal(SIGSEGV, cleanup);

#ifdef SIGBUS
	signal(SIGBUS, cleanup);
#endif
#ifdef SIGEMT
	signal(SIGEMT, cleanup);
#endif
#ifdef SIGPOLL
	signal(SIGPOLL, cleanup);
#endif
#ifdef SIGSYS
	signal(SIGSYS, cleanup);
#endif
#ifdef SIGTRAP
	signal(SIGTRAP, cleanup);
#endif
#ifdef SIGVTALRM
	signal(SIGVTALRM, cleanup);
#endif
#ifdef SIGXCPU
	signal(SIGXCPU, cleanup);
#endif
#ifdef SIGXFSZ
	signal(SIGXFSZ, cleanup);
#endif
}

static void lockNow( int )
{
	forceTimeout();
	// SIGUSR1 forces a lock
	signal(SIGUSR1, lockNow);
}

static void cleanup( int id )
{
    if ( mode == MODE_INSTALL )
	{
	    remove( pidFile );
	    if (id != SIGPIPE)
		XSetScreenSaver( qt_xdisplay(), xs_timeout, xs_interval,
				 xs_prefer_blanking, xs_allow_exposures );
	}
    exit(1);
}

void usage( char *name )
{
	printf( glocale->translate(
	   "Usage: %s -install|-setup|-test|-desc|-preview wid|-inroot\n"\
	   "       [-corners xxxx] [-delay num] [-lock] [-nice num]\n"), name );
	printf( glocale->translate(
	"  -corners xxxx     Placing cursor in corner performs action:\n"\
	"                     x = i  no action (ignore)\n"\
	"                     x = s  save screen\n"\
	"                     x = l  lock screen\n"\
	"                    order: top-left, top-right, bottom-left, bottom-right\n"\
	"  -delay num        Amount of idle time before screen saver\n"\
	"                     starts  (default 10min)\n"\
	"  -desc             Print the screen saver's description to stdout\n"\
	"  -install          Install screen saver\n"\
	"  -lock             Require password to stop screen saver\n"\
	"  -nice num         Run with specified nice value\n\n"\
	"  -preview wid      Run in the specified XWindow\n"\
	"  -inroot           Run in the root window\n"\
	"  -setup            Setup screen saver\n"\
	"  -test             Invoke the screen saver immediately\n"));
	exit(1);
}

