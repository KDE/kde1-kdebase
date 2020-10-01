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
#include <sys/types.h>
#include <sys/wait.h>
#include <kprocess.h>
#include <qapp.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <qkeycode.h>
#include <qdialog.h>
#include <qpushbt.h>
#include <qbitmap.h>
#include <qpainter.h>

#include "xautolock.h"
#include "saver.h"
#include "main.h"
#include "main.moc"
#include <kmsgbox.h>

KLocale *glocale;

extern "C" {
  extern void startScreenSaver( Drawable d );
  extern void stopScreenSaver();
  extern int  setupScreenSaver();
  extern const char *getScreenSaverName();
  extern void exposeScreenSaver( int x, int y, int width, int height );
}

extern void initPasswd();
int mode = MODE_NONE;
static bool lock = false, passOk = false;
static bool canGetPasswd;
static bool lockOnce = false;
static bool only1Time = false;
static int xs_timeout, xs_interval, xs_prefer_blanking, xs_allow_exposures;
static QString pidFile;
static KPasswordDlg *passDlg = NULL;
static QWidget *saverWidget = NULL;
static int desktopWidth = 0, desktopHeight = 0;
extern char *ProgramName;
extern Bool allowroot;

static bool grabInput( QWidget *w );
static void releaseInput();
static void destroySaverWindow( QWidget *w );
static void lockNow( int );
static void cleanup( int );
static void catchSignals();
static void usage( char *name );

ssApp *globalKapp;

//----------------------------------------------------------------------------

ssApp::ssApp( int &argc, char **argv ) : KApplication( argc, argv )
{
	KConfig kssConfig( kapp->kde_configdir() + "/kssrc", 
	                   kapp->localconfigdir() + "/kssrc" );
	kssConfig.setGroup( "kss" );
	stars = kssConfig.readBoolEntry( "PasswordAsStars", true );
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
				passDlg->move( (desktopWidth - passDlg->width())/2,
                                (desktopHeight - passDlg->height())/2 );
				passDlg->show();
			}
			return TRUE;
		}
		else if ( event->type == VisibilityNotify )
		{
			if ( event->xvisibility.state != VisibilityUnobscured)
			{
				if ( !passDlg )
                {
					saverWidget->raise();
                    QApplication::flushX();
                }
			}
		}
        else if ( event->type == ConfigureNotify )
        {
            if ( !passDlg )
            {
                saverWidget->raise();
                QApplication::flushX();
            }
        }
        else if ( event->type == Expose )
        {
            if (!passDlg)
            {
                exposeScreenSaver( event->xexpose.x,
                        event->xexpose.y,
                        event->xexpose.width,
                        event->xexpose.height );
            }
		}
	}

	return FALSE;
}

void ssApp::slotPassOk()
{
	passOk = TRUE;
    passDlg->hide();
	QApplication::flushX();
	delete passDlg;
	passDlg = NULL;
	qApp->exit_loop();
	QApplication::flushX();
}

void ssApp::slotPassCancel()
{
	passOk = FALSE;
    passDlg->hide();
	QApplication::flushX();
	delete passDlg;
	passDlg = NULL;
	grabInput( saverWidget );
	QApplication::flushX();
}

//----------------------------------------------------------------------------

static bool grabInput( QWidget *w)
{
	int rv = XGrabKeyboard( qt_xdisplay(), QApplication::desktop()->winId(),
                True, GrabModeAsync, GrabModeAsync, CurrentTime );

    if (rv == AlreadyGrabbed)
    {
        return false;
    }

	rv = XGrabPointer( qt_xdisplay(), QApplication::desktop()->winId(), True,
			ButtonPressMask
			| ButtonReleaseMask | EnterWindowMask | LeaveWindowMask
			| PointerMotionMask | PointerMotionHintMask | Button1MotionMask
			| Button2MotionMask | Button3MotionMask | Button4MotionMask
			| Button5MotionMask | ButtonMotionMask | KeymapStateMask,
			GrabModeAsync, GrabModeAsync, None, w->cursor().handle(),
			CurrentTime );

    if (rv == AlreadyGrabbed)
    {
        return false;
    }

    return true;
}

static void releaseInput()
{
	XUngrabKeyboard( qt_xdisplay(), CurrentTime );
	XUngrabPointer( qt_xdisplay(), CurrentTime );
}

static QWidget *createSaverWindow()
{
	QWidget *w;

	// WStyle_Customize sets override_redirect
	w = new QWidget( NULL, "", WStyle_Customize | WStyle_NoBorder );

	/* set NoBackground so that the saver can capture the current
	 * screen state if necessary
	 */
	w->setBackgroundMode( QWidget::NoBackground );

	XSetWindowAttributes attr;
	attr.event_mask = KeyPressMask | ButtonPressMask | MotionNotify |
		 VisibilityChangeMask | ExposureMask; // | StructureNotifyMask;
	XChangeWindowAttributes(qt_xdisplay(), w->winId(), CWEventMask, &attr);

	QBitmap bm( 1, 1, TRUE );
	QCursor c( bm, bm );
	w->setCursor( c );

	if (grabInput( w ) == false)
    {
        // The grab failed - we can't save the window now.
        destroySaverWindow(w);
        w = 0;
    }

	return w;
}

static void destroySaverWindow( QWidget *w )
{
	releaseInput();
	delete w;
}

//----------------------------------------------------------------------------

static QString lockName(QString s)
{
	// note that changes in the pidFile name have also to be done
	// in kdebase/kcontrol/display/scrnsave.cpp
	QString name = getenv( "HOME" );
	name += "/.kss-" + s + ".pid.";
	char ksshostname[200];
	gethostname(ksshostname, 200);
	name += ksshostname;
	return name;
}

static int getLock(QString type)
{
	QString lockFile = lockName(type);
	int pid = -1;

	FILE *fp;
	if ( (fp = fopen( lockFile, "r" ) ) != NULL )
	{
		fscanf( fp, "%d", &pid );
		fclose( fp );
	}

	return pid;
}

static void killProcess(int pid)
{
	if ( pid != getpid() && pid > 1 )
	{
		if( kill( pid, SIGTERM ) == 0 ) 
			sleep(1);
	}
}

static void setLock(QString type)
{
	FILE *fp;
	pidFile = lockName(type);
	int pid = getLock( type );

	killProcess( pid );

	if ( (fp = fopen( pidFile, "w" ) ) != NULL )
	{
		// on some systems it's long, on some int, so a cast may help
		fprintf( fp, "%ld\n", static_cast<long>(getpid()) );
		fclose( fp );
	}
}

/* Verify, if kde1-kcheckpass is able to verify passwords.
 * I cannot use KProcess here, as it needs ProcessEvents */
static bool canReadPasswdDatabase()
{
	KProcess chkpass;
	QString kcp_binName = "";
	kcp_binName += KApplication::kde_bindir();
	kcp_binName += "/kde1-kcheckpass";
	chkpass.clearArguments();
	chkpass << kcp_binName;
	bool ret = chkpass.start(KProcess::DontCare, KProcess::Stdin);
	if (ret == false)
          return false;

	chkpass.closeStdin();
	int timeout = 1000;
	while ( timeout != 0 ) {
	  if (! chkpass.isRunning() )
	    break;
	  else {
	    globalKapp->processEvents();
	    timeout--;
	    usleep(10000);
	  }
	}
	
	int canRead = ( chkpass.normalExit() && (chkpass.exitStatus() != 2) );
	return canRead;
}

//----------------------------------------------------------------------------

int main( int argc, char *argv[] )
{
	srand( getpid() );  /* seed the random generator */

#ifndef HAVE_PAM // If we have PAM we don't need suid
	// drop root privileges temporarily
#ifdef HAVE_SETEUID
	seteuid(getuid());
#else
        setreuid(-1, getuid());
#endif // HAVE_SETEUID
#endif // HAVE_PAM

	Window saveWin = 0;
	int timeout = 600;
	ProgramName = argv[0];
	ssApp a( argc, argv );
        globalKapp = &a;
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

	    /* if it's the last one, check if we expect a value: */
	    if( i >= (argc-1) ) {
	        switch (parameter) 
		    {
		    case preview:
		    case delay:
		    case corners:
		    case arg_nice:
		        usage( argv[0] ); // won't return
		        break;
		    default:
		        break;
		    }
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
		    if( timeout == 0 )
			only1Time = true;
		    else if( timeout < 60 )
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

#ifndef HAVE_PAM
	// regain root privileges
#ifdef HAVE_SETEUID
	seteuid(0);
#else
        setreuid(-1, 0);
#endif // HAVE_SETEUID
#endif // HAVE_PAM

	initPasswd();

#ifndef HAVE_PAM
	// ... and drop them again before doing anything important
	setuid(getuid());
#endif // HAVE_PAM

	// now check, if I can verify passwords (might be a problem
	// only with shadow passwords, due to missing SUID on
	// kde1-kcheckpass program.
#ifdef HAVE_SHADOW
        canGetPasswd = canReadPasswdDatabase();
#else
	canGetPasswd = true;
#endif

    // Get the size of the desktop
    XWindowAttributes attr;
    if (XGetWindowAttributes(qt_xdisplay(), RootWindow(qt_xdisplay(),
        qt_xscreen()), &attr) == 0)
    {
        debug("Failed getting Root window size");
    }
    else
    {
        desktopWidth = attr.width;
        desktopHeight = attr.height;
    }

	catchSignals();
	if ( mode == MODE_INSTALL )
	{
		if (!canGetPasswd && lock) {
			QString tmp = glocale->translate(
			              "Warning: You won't be able to lock the screen!\n\n"
			              "Your system uses shadow passwords.\n"
			              "Please contact your system administrator.\n"
			              "Tell him that you need suid for the kde1-kcheckpass program!");
			KMsgBox::message(NULL, 
				 glocale->translate("Shadow Passwords"), 
				 tmp, KMsgBox::EXCLAMATION);
		}
		setLock("install");

		XGetScreenSaver( qt_xdisplay(), &xs_timeout, &xs_interval,
				&xs_prefer_blanking, &xs_allow_exposures );
		XSetScreenSaver( qt_xdisplay(), 0, xs_interval, xs_prefer_blanking,
			xs_allow_exposures );

        initAutoLock();
			
		while ( 1 )
		{
			if ( waitTimeout( timeout ) == FORCE_LOCK )
				lockOnce = TRUE;

			// if another saver is in test-mode, kill it
			killProcess(getLock("test"));

			saverWidget = createSaverWindow();

            if (saverWidget)
            {
                saverWidget->setFixedSize(desktopWidth, desktopHeight);
                saverWidget->move( 0, 0 );
                saverWidget->show();
                saverWidget->raise();
                QApplication::flushX();

                saveWin = saverWidget->winId();

                startScreenSaver( saveWin );
                a.enter_loop();
                stopScreenSaver();

                destroySaverWindow( saverWidget );

                lockOnce = FALSE;
                if( only1Time ) 
                    break;
            }
		}

        cleanupAutoLock();
	}
	else if ( mode == MODE_TEST )
	{
		if( lock ) {
			fprintf(stderr, glocale->translate(
	                        "\nplease don't use -test together with -lock.\n"
			        "use klock instead.\n\n") );
			exit(1);
		}
		setLock("test");
		saverWidget = createSaverWindow();
        if (saverWidget)
        {
            saverWidget->setFixedSize(desktopWidth, desktopHeight);
            saverWidget->move( 0, 0 );
            saverWidget->show();
            saverWidget->raise();
            QApplication::flushX();

            saveWin = saverWidget->winId();

            startScreenSaver( saveWin );
            a.enter_loop();
            stopScreenSaver();

            destroySaverWindow( saverWidget );
        }
	}
	else if ( mode == MODE_PREVIEW )
	{
		char mode[32];
		sprintf(mode, "preview%ld", (unsigned long) saveWin);
		setLock(mode);
		startScreenSaver( saveWin );
		a.exec();
	}
	else if ( mode == MODE_SETUP )
	{
		setupScreenSaver();
	}
	else
		usage( argv[0] );

    if ( !pidFile.isEmpty() )
    {
        remove( pidFile );
    }
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
    if ( !pidFile.isEmpty() )
    {
        remove( pidFile );
    }
	if ( mode == MODE_INSTALL )
	{
		if (id != SIGPIPE)
        {
			XSetScreenSaver( qt_xdisplay(), xs_timeout, xs_interval,
			                 xs_prefer_blanking, xs_allow_exposures );
            cleanupAutoLock();
        }
	}
	exit(1);
}

void usage( char *name )
{
	printf( glocale->translate(
	   "Usage: %s -install|-setup|-test|-desc|-preview wid|-inroot\n"\
	   "       [-corners xxxx] [-delay num] [-lock] [-allow-root] [-nice num]\n"), name ); 
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
	"  -allow-root       Accept root password to unlock\n"\
	"  -nice num         Run with specified nice value\n\n"\
	"  -preview wid      Run in the specified XWindow\n"\
	"  -inroot           Run in the root window\n"\
	"  -setup            Setup screen saver\n"\
	"  -test             Invoke the screen saver immediately\n"));
	exit(1);
}

