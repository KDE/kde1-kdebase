//-----------------------------------------------------------------------------
//
// KDE Help main
//
// (c) Martin R. Jones 1996
//

#include <qfileinf.h>

#include "helpwin.h"
#include <signal.h>
#include <sys/stat.h>
#include <qmsgbox.h>
#include <kapp.h>
#include <drag.h>
#include <klocale.h>
#include <kwm.h>
#include <time.h>
#include "error.h"
#include "khelp.h"
#include "mainwidget.h"

#include <X11/Xlib.h>
#include <X11/Xatom.h>

#ifdef __FreeBSD__
#include <floatingpoint.h>
#endif

static void cleanup( int );
void catchSignals();

static int msgqid = -1;
static QString pidFile;

//-----------------------------------------------------------------------------
// timer used to monitor msg queue for request for new window

class Timer : public QObject
{
public:
	Timer()
	{
		startTimer( 100 );
	}

protected:
	virtual void timerEvent( QTimerEvent * );
};

void Timer::timerEvent( QTimerEvent * )
{
    KHelpMsg buf, retbuf;
    KHelpMain *helpWin;

    if ( buf.recv( msgqid, 1L ) != -1 )
    {
	printf( "got request: %s\n", buf.getMsg() );

	retbuf.setType( 2L );
	retbuf.send( msgqid );

	helpWin = new KHelpMain;

	QString tmp = buf.getMsg();

	QString url = strtok( tmp.data(), " " );
	QString props = strtok( NULL, " " );

	if ( !strchr( url, ':' ) )
	{
	    url = "file:";
	    url += buf.getMsg();
	}

	if ( helpWin->openURL( url ) )
		delete helpWin;
	else
	{
	    if ( !props.isEmpty() )
		KWM::setProperties( helpWin->winId(), props );
	    helpWin->show();
	}
    }
}

void errorHandler( int type, char *msg )
{
	QApplication::setOverrideCursor( arrowCursor );

	QMessageBox::message( klocale->translate("Error"), 
			      msg, 
			      klocale->translate("OK") );

	QApplication::restoreOverrideCursor();

	if ( type == ERR_FATAL )
	{
		if (msgqid >= 0)
			msgctl( msgqid, IPC_RMID, 0 );
		remove( pidFile );
		exit(1);
	}
}

static QString displayName()
{
  // note: We can not rely on DISPLAY. If KDE is started by
  // KDM, DISPLAY will be something like ":0", but this is
  // not unique if we start KDE several times in a network

  QString d = QString(getenv("DISPLAY"));

  int i = d.find( ':' );
  if ( i != -1 )
    d[i] = '_';
  if (i==0)
    {
      // we are running local, so add the hostname
      char name[25];

      if (gethostname(name, 25) == 0)
	d = name + d;
    }

  if ( d.find( '.' ) == -1 )
    d += ".0";

  return d;
}

//-----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
	int i;
	QString url, initDoc;
	FILE *fp;

	for ( i = 1; i < argc; i++ )
	{
		if ( argv[i][0] == '-' )
		{
            if ( ( strcasecmp( argv[i], "-caption" ) == 0 ) ||
                ( strcasecmp( argv[i], "-icon" ) == 0 ) ||
                ( strcasecmp( argv[i], "-miniicon" ) == 0 ) )
            {
                i++;
            }
            continue;
		}

		initDoc = argv[i];
	}

	if ( initDoc.isEmpty() )
	{
		initDoc = "file:";
		initDoc += kapp->kde_htmldir().copy();
		initDoc += "/default/kdehelp/main.html";
	}

	url = initDoc;

	if ( !strchr( url, ':' ) )
	{
		if ( initDoc[0] == '.' || initDoc[0] != '/' )
		{
			QFileInfo fi( initDoc );
			initDoc = fi.absFilePath();
		}

		url = "file:";
		url += initDoc;
	}

	// create data directory if necessary
	QString p = KApplication::localkdedir();
	QString rcDir = p + "/share/apps";
	if ( access( rcDir, F_OK ) )
		mkdir( rcDir.data(), 0740 );

	rcDir += "/kdehelp";
	if ( access( rcDir, F_OK ) )
		mkdir( rcDir.data(), 0740 );

	pidFile.sprintf("%s/kdehelp%s.pid", rcDir.data(), displayName().data());

	// if there is a pidFile then this is not the first instance of kdehelp
	if ( ( fp = fopen( pidFile, "r" ) ) != NULL )
	{
		KHelpMsg buf;
		int pid;
		QString msg = url;
		buf.setType( 1L );
		buf.setMsg( msg );
		fscanf( fp, "%d %d", &pid, &msgqid );
		// if this fails I assume that the pid file is left over from bad exit
		// and continue on
		//
		if ( buf.send( msgqid ) != -1)
		{
			// if we don't receive a reply within 3secs assume previous
			// instance of kdehelp died an unnatural death.
			// How should this stuff be handled properly?
			//
			time_t start = time(NULL);
			while ( time(NULL) - start < 3 )
			{
				if (buf.recv( msgqid, 2L, IPC_NOWAIT ) != -1)
				{
					fclose( fp );
					exit(0);
				}
			}
			msgctl( msgqid, IPC_RMID, NULL );
		}
		fclose( fp );
	}

	// This is the first instance so create a pid/msgqid file
	key_t key = ftok( getenv( "HOME" ), (char)rand() );
	msgqid = msgget( key, IPC_CREAT | 0600 );

	fp = fopen( pidFile, "w" );
	fprintf( fp, "%ld %d\n", (long)getpid(), msgqid );
	fclose( fp );

	// so that everything is cleaned up
	catchSignals();

	Timer *timer = new Timer;

	// error handler for info and man stuff
	Error.SetHandler( (void (*)(int, const char *))errorHandler );

	KApplication a( argc, argv, "kdehelp" );

	KHelpMain *helpWin;

	if ( a.isRestored() )
	{
	    int n = 1;
	    while ( KTopLevelWidget::canBeRestored( n ) )
	    {
		helpWin = new KHelpMain;
		helpWin->restore( n );
		n++;
	    }

	    a.exec();
	}
	else
	{
	    helpWin = new KHelpMain;

	    if ( !helpWin->openURL( url ) )
	    {
		    helpWin->show();

		    a.exec();
	    }
	}

	delete timer;

	msgctl( msgqid, IPC_RMID, 0 );

	remove( pidFile );
}

// make sure the pid file is cleaned up when exiting unexpectedly.
//
void catchSignals()
{
//	signal(SIGHUP, cleanup);		/* Hangup */
	signal(SIGINT, cleanup);		/* Interrupt */
	signal(SIGTERM, cleanup);		/* Terminate */
//	signal(SIGCHLD, cleanup);

	signal(SIGABRT, cleanup);
	signal(SIGALRM, cleanup);
	signal(SIGFPE, cleanup);
	signal(SIGILL, cleanup);
	signal(SIGPIPE, cleanup);
	signal(SIGQUIT, cleanup);
//	signal(SIGSEGV, cleanup);

#ifdef SIGBUS
	signal(SIGBUS, cleanup);
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

// remove pid file
static void cleanup( int sig )
{
	printf( "cleanup: signal = %d\n", sig );
	if (msgqid >= 0)
		msgctl( msgqid, IPC_RMID, 0 );
	remove( pidFile );
    if ( sig == SIGPIPE )
    {
        // Broken pipe is probably broken X connection.
        // This causes problems with global Qt destructors.
        _exit(1);
    }
    exit(0);
}

