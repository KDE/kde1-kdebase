//-----------------------------------------------------------------------------
//
// KDE Display screen saver setup module
//
// Copyright (c)  Martin R. Jones 1996
//
// Converted to a kcc module by Matthias Hoelzer 1997
//

#include <kprocess.h>

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <qgrpbox.h>
#include <qbttngrp.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qpushbt.h>
#include <qfiledlg.h>
#include <qradiobt.h>
#include <qchkbox.h>
#include <qslider.h>
#include <kapp.h>
#include <stdlib.h>
#include <X11/Xlib.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "kcolordlg.h"
#include "scrnsave.h"
#include "scrnsave.moc"

#include <kiconloader.h>
#include <kcontrol.h>


#define NO_SCREENSAVER klocale->translate("(none)")
#define SCREENSAVER_DIR	"/usr/local/kde/bin"
#define CORNER_SIZE		15

CornerButton::CornerButton( QWidget *parent, int num, char _action )
	: QLabel( parent )
{
	popupMenu.insertItem(  klocale->translate("Ignore"), 'i' );
	popupMenu.insertItem(  klocale->translate("Save"), 's' );
	popupMenu.insertItem(  klocale->translate("Lock"), 'l' );

	connect( &popupMenu, SIGNAL( activated( int ) ),
		SLOT( slotActionSelected( int ) ) );

	number = num;
	action = _action;

	setActionText();

	setAlignment( AlignCenter );
}

void CornerButton::setActionText()
{
	switch ( action )
	{
		case 'i':
			setText( "" );
			break;

		case 's':
			setText( "s" );
			break;

		case 'l':
			setText( "l" );
			break;
	}
}

void CornerButton::mousePressEvent( QMouseEvent *me )
{
	QPoint p = mapToGlobal( me->pos() );

	popupMenu.popup( p );
}

void CornerButton::slotActionSelected( int a )
{
	action = a;
	setActionText();
	emit cornerAction( number, action );
}


int discardError(Display *, XErrorEvent *)
{
	return 0;
}

KScreenSaver::KScreenSaver( QWidget *parent, int mode, int desktop )
	: KDisplayModule( parent, mode, desktop )
{
KIconLoader iconLoader;

	readSettings();

	setName(  klocale->translate("Screen Saver") );

	// if we are just initialising we don't need to create setup widget
	if ( mode == Init )
	{
		ssSetup = 0;
		ssPreview = 0;
		return;
	}

	ssSetup = new KProcess;
	CHECK_PTR(ssSetup);
	connect(ssSetup, SIGNAL(processExited(KProcess *)),
	    this, SLOT(slotSetupDone(KProcess *)));

	ssPreview = new KProcess;
	CHECK_PTR(ssPreview);
	connect(ssPreview, SIGNAL(processExited(KProcess *)),
	    this, SLOT(slotPreviewExited(KProcess *)));

	findSavers();

	QPixmap p = iconLoader.loadIcon("monitor.xpm");

	QLabel *label;

	label = new QLabel( this );
	label->setPixmap( p );
	label->setGeometry( 135, 15, label->sizeHint().width(),
					 label->sizeHint().height() );

	monitor = new KSSMonitor( label );
	monitor->setGeometry( 20, 10, 157, 111 );
	monitor->setBackgroundColor( black );

	CornerButton *corner = new CornerButton( monitor, 0, cornerAction[0] );
	corner->setGeometry( 0, 0, CORNER_SIZE, CORNER_SIZE );
	connect( corner, SIGNAL( cornerAction( int, char ) ),
			SLOT( slotCornerAction( int, char ) ) );

	corner = new CornerButton( monitor, 1, cornerAction[1] );
	corner->setGeometry( monitor->width()-CORNER_SIZE, 0, CORNER_SIZE, CORNER_SIZE );
	connect( corner, SIGNAL( cornerAction( int, char ) ),
			SLOT( slotCornerAction( int, char ) ) );

	corner = new CornerButton( monitor, 2, cornerAction[2] );
	corner->setGeometry( 0, monitor->height()-CORNER_SIZE, CORNER_SIZE, CORNER_SIZE );
	connect( corner, SIGNAL( cornerAction( int, char ) ),
			SLOT( slotCornerAction( int, char ) ) );

	corner = new CornerButton( monitor, 3, cornerAction[3] );
	corner->setGeometry( monitor->width()-CORNER_SIZE, monitor->height()-CORNER_SIZE, CORNER_SIZE, CORNER_SIZE );
	connect( corner, SIGNAL( cornerAction( int, char ) ),
			SLOT( slotCornerAction( int, char ) ) );

	QGroupBox *group = new QGroupBox(  klocale->translate("Screen Saver"), this );
	group->setGeometry( 195, 190, 260, 130 );

	ssList = new QListBox( group );
	ssList->insertItem( NO_SCREENSAVER, 0 );
	ssList->setCurrentItem( 0 );
	ssList->setGeometry( 15, 20, 135, 90 );
	getSaverNames();
//	ssList->insertStrList( &saverNames );
	QStrListIterator it( *saverList );
	for ( int i = 1; it.current(); ++it )
	{
		ssList->insertItem( saverNames.at( i - 1 ), i );
		if ( saverFile == it.current() )
			ssList->setCurrentItem( i );
		i++;
	}
	ssList->setTopItem( ssList->currentItem() );
	connect( ssList, SIGNAL( highlighted( int ) ),
			SLOT( slotScreenSaver( int ) ) );

	setupBt = new QPushButton(  klocale->translate("Setup..."), group );
	setupBt->setGeometry( 160, 20, 90, 25 );
	connect( setupBt, SIGNAL( clicked() ), SLOT( slotSetup() ) );

	testBt = new QPushButton(  klocale->translate("Test"), group );
	testBt->setGeometry( 160, 55, 90, 25 );
	connect( testBt, SIGNAL( clicked() ), SLOT( slotTest() ) );

	QPushButton *button = new QPushButton(  klocale->translate("Help"), group );
	button->setGeometry( 160, 90, 90, 25 );
	connect( button, SIGNAL( clicked() ), SLOT( slotHelp() ) );

	group = new QGroupBox(  klocale->translate("Settings"), this );
	group->setGeometry( 15, 190, 165, 130 );

	label = new QLabel(  klocale->translate("Wait:"), group );
	label->setGeometry( 15, 20, 40, 20 );

	waitEdit = new QLineEdit( group );
	waitEdit->setGeometry( 55, 20, 45, 20 );
	QString str;
	str.setNum( xtimeout/60 );
	waitEdit->setText( str );
	connect( waitEdit, SIGNAL( textChanged( const char * ) ),
			SLOT( slotTimeoutChanged( const char * ) ) );

	label = new QLabel(  klocale->translate("min"), group );
	label->setGeometry( 105, 20, 40, 20 );

	cb = new QCheckBox(  klocale->translate("Requires Password"), group );
	cb->setGeometry( 15, 50, 145, 20 );
	cb->setChecked( lock );
	connect( cb, SIGNAL( toggled( bool ) ), SLOT( slotLock( bool ) ) );

	label = new QLabel(  klocale->translate("Priority:"), group );
	label->setGeometry( 15, 75, 80, 20 );

	label = new QLabel(  klocale->translate("High"), group );
	label->setGeometry( 15, 100, 40, 20 );

	label = new QLabel(  klocale->translate("Low"), group );
	label->setGeometry( 115, 100, 45, 20 );

	prioritySlider = new QSlider( QSlider::Horizontal, group );
	prioritySlider->setGeometry( 50, 100, 60, 20 );
	prioritySlider->setRange( 0, 20 );
	prioritySlider->setSteps( 5, 10 );
	prioritySlider->setValue( priority );
	connect( prioritySlider, SIGNAL( valueChanged(int) ),
		SLOT( slotPriorityChanged(int) ) );

//	connect( &timer, SIGNAL( timeout() ), SLOT( slotSetupTimeout() ) );

	// I have to call show() here, otherwise the screensaver
	// does not get the correct size information.
	show();

	setMonitor();
}

KScreenSaver::~KScreenSaver()
{
    delete ssPreview; // CC: This also terminates the preview process..
    delete ssSetup;
}

void KScreenSaver::readSettings( int )
{
	// get X screen saver attribs
	XGetScreenSaver( qt_xdisplay(), &xtimeout, &xinterval,
	       &xprefer_blanking, &xallow_exposures );

	QString str;

	saverFile = NO_SCREENSAVER;
	saverLocation = SCREENSAVER_DIR;

	KConfig *config = kapp->getConfig();
	config->setGroup( "ScreenSaver" );

	str = config->readEntry( "Location" );
	if ( !str.isNull() )
		saverLocation = str;
	else
	{
		saverLocation = kapp->kdedir();
		saverLocation += "/bin";
	}

	str = config->readEntry( "Saver" );
	if ( !str.isNull() )
		saverFile = str;

	str = config->readEntry( "Timeout" );
	if ( !str.isNull() )
		xtimeout = atoi( str );
	
	if ( xtimeout == 0 )
		xtimeout = 60;

	str = config->readEntry( "Lock" );
	if ( !str.isNull() && str.find( "yes" ) == 0 )
		lock = TRUE;
	else
		lock = FALSE;

	str = config->readEntry( "Priority" );
	if ( !str.isNull() )
		priority = atoi( str );
	else
		priority = 0;

	if ( priority < 0 )
		priority = 0;
	else if ( priority > 19 )
		priority = 19;

	str = config->readEntry( "CornerAction" );
	if ( !str.isNull() )
		strncpy( cornerAction, str, 4 );
	else
		strcpy( cornerAction, "iiii" );
	
	cornerAction[4] = '\0';
}

void KScreenSaver::writeSettings()
{
	if ( !changed )
		return;

	KConfig *config = kapp->getConfig();
	config->setGroup( "ScreenSaver" );

	config->writeEntry( "Saver", saverFile );

	QString str;
	str.setNum( xtimeout );
	config->writeEntry( "Timeout", str );

	config->writeEntry( "Lock", lock ? "yes" : "no" );

	str.detach();
	str.setNum( priority );
	config->writeEntry( "Priority", str );

	config->writeEntry( "CornerAction", cornerAction );

	changed = FALSE;
}

void KScreenSaver::findSavers()
{
	static QDir d( saverLocation );

	d.setFilter( QDir::Executable | QDir::Files );

	saverList = d.entryList( "*.kss" );
}

void KScreenSaver::getSaverNames()
{
	KConfig *config = kapp->getConfig();
	config->setGroup( "Saver Names" );

	saverNames.clear();

	QStrListIterator it( *saverList );
	for ( ; it.current(); ++it )
	{
		QString name = config->readEntry( it.current() );

		if ( name.isEmpty() )
		{
			char buffer[80];
			QString cmd = saverLocation + '/' + it.current() + " -desc";
			FILE *fp = popen( cmd, "r");
			if ( fp )
			{
				fgets( buffer, 80, fp );
				if ( strchr( buffer, '\n' ) )
					*strchr( buffer, '\n' ) = '\0';
				pclose( fp );
				name = buffer;
				config->writeEntry( it.current(), buffer );
			}
			else
				name = "";
		}

		saverNames.append( name );
	}
}

void KScreenSaver::slotApply()
{
	apply();
	writeSettings();
}

void KScreenSaver::apply( bool force )
{
	if ( !changed && !force )
		return;

	if ( saverFile == NO_SCREENSAVER )
	{
		QString pidFile;
		pidFile = getenv( "HOME" );
		pidFile += "/.kss.pid";
		FILE *fp;
		if ( ( fp = fopen( pidFile, "r" ) ) != NULL )
		{
			int pid;
			fscanf( fp, "%d", &pid );
			fclose( fp );
			kill( pid, SIGTERM );
		}
		XSetScreenSaver( qt_xdisplay(), 0, xinterval,
			xprefer_blanking, xallow_exposures );
		return;
	}

	int pid = fork();

	if ( pid == 0 )
	{
		char *slock = "-lock";
		if ( !lock )
			slock = NULL;
		QString sDelay;
		sDelay.setNum( xtimeout/60 );
		QString sPriority;
		sPriority.setNum( priority );
		QString path = saverLocation + '/' + saverFile;

		execl( path.data(), path.data(), "-delay", sDelay.data(), "-install",
				"-corners", cornerAction, "-nice", sPriority.data(),
				slock, NULL );
		exit(1);
	}
}

void KScreenSaver::setMonitor()
{
	if (ssPreview->isRunning())
	    // CC: this will automatically cause a "slotPreviewExited"
	    // when the viewer exits
	    ssPreview->kill( );
	else
	    slotPreviewExited(ssPreview);
}

void KScreenSaver::slotPreviewExited(KProcess *)
{
    QString path = saverLocation + '/' + saverFile;
    QString id;

    monitor->setBackgroundColor( black );
    monitor->erase();

    if ( saverFile == NO_SCREENSAVER )
	return;

    id.setNum( monitor->winId() );

    ssPreview->clearArguments();
    ssPreview->setExecutable(path.data());
    *ssPreview << "-preview" << id.data();
    ssPreview->start();
}

void KScreenSaver::slotScreenSaver( int indx )
{
	if ( indx == 0 )
	{
		saverFile = NO_SCREENSAVER;
		setupBt->setEnabled( FALSE );
		testBt->setEnabled( FALSE );
	}
	else
	{
		QStrListIterator it( *saverList );
		saverFile = it += (indx - 1);
		if (!ssSetup->isRunning())
			setupBt->setEnabled( TRUE );
		testBt->setEnabled( TRUE );
	}

	setMonitor();

	changed = TRUE;
}

void KScreenSaver::slotSetup()
{
	QString path;

	if ( saverFile == NO_SCREENSAVER )
	    return;

	if (ssSetup->isRunning())
	    return;
	
	setupBt->setEnabled( FALSE );
	kapp->flushX();

	path =  saverLocation + '/' + saverFile;

//	connect(ssSetup, SIGNAL(processExited(KProcess *)),
//	    this, SLOT(slotSetupDone(KProcess *)));
	ssSetup->clearArguments();
	ssSetup->setExecutable(path);
	*ssSetup << "-setup";
	ssSetup->start();
}

void KScreenSaver::slotTest()
{
	KProcess proc;

	QString path = saverLocation + '/' + saverFile;

	testBt->setEnabled( FALSE );
	kapp->flushX(); // CC: draw the disabled button _now_ 

	proc.clearArguments();
	proc.setExecutable(path);
	proc << "-test";
	proc.start(KProcess::Block);

	/* CC: now eliminate all events that have got accumulated while
	    we were sleeping... */
	kapp->processEvents();
	debug("ENABLING BUTTON!!!");
	testBt->setEnabled(TRUE);
}

void KScreenSaver::slotTimeoutChanged( const char *to )
{
	xtimeout = atoi( to ) * 60;

	if ( xtimeout <= 0 )
		xtimeout = 60;
	changed = TRUE;
}

void KScreenSaver::slotLock( bool l )
{
	lock = l;
	changed = TRUE;
}

void KScreenSaver::slotPriorityChanged( int val )
{
	if ( val != priority )
		changed = TRUE;
	
	priority = val;

	if ( priority > 19 )
		priority = 19;
}

void KScreenSaver::slotSetupDone(KProcess *)
{
	debug("SETUP DONE!!!");
	setMonitor();
	setupBt->setEnabled( TRUE );
}

void KScreenSaver::slotHelp()
{
	kapp->invokeHTMLHelp( "kdisplay/kdisplay-4.html", "" );
}

void KScreenSaver::slotCornerAction( int num, char action )
{
	cornerAction[num] = action;
	changed = TRUE;
}

void KScreenSaver::loadSettings()
{
  ssList->clear();  
  ssList->insertItem( NO_SCREENSAVER, 0 );

  readSettings();

  findSavers();
  getSaverNames();

  QStrListIterator it( *saverList );
  for ( int i = 1; it.current(); ++it )
    {
	ssList->insertItem( saverNames.at( i - 1 ), i );
	if ( saverFile == it.current() )
           ssList->setCurrentItem( i );
        i++;
    }
  ssList->setTopItem( ssList->currentItem() );

  QString str;
  str.setNum( xtimeout/60 );
  waitEdit->setText( str );

  cb->setChecked( lock );

  prioritySlider->setValue( priority );
}

void KScreenSaver::applySettings()
{
  writeSettings();
  apply(TRUE);
}