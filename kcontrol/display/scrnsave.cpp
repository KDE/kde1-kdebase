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
#include <qlayout.h>
#include <kapp.h>
#include <stdlib.h>
#include <X11/Xlib.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <kiconloader.h>
#include <kcontrol.h>

#include "kcolordlg.h"
#include "scrnsave.h"
#include "scrnsave.moc"

#define SCREENSAVER_DIR	"/usr/local/kde/bin"
#define CORNER_SIZE		15

CornerButton::CornerButton( QWidget *parent, int num, char _action )
	: QLabel( parent )
{
	popupMenu.insertItem(  i18n("Ignore"), 'i' );
	popupMenu.insertItem(  i18n("Save Screen"), 's' );
	popupMenu.insertItem(  i18n("Lock Screen"), 'l' );

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
	kssConfig = new KConfig( kapp->kde_configdir() + "/kssrc", 
		                 kapp->localconfigdir() + "/kssrc" );
	kssConfig->setGroup( "kss" );

	readSettings();
	
	//debug("KScreenSaver::KScreenSaver");

	setName(  i18n("Screen Saver") );

	// if we are just initialising we don't need to create setup widget
	if ( mode == Init )
	{
		ssSetup = 0;
		ssPreview = 0;
		apply( true );
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
	
	QGridLayout *topLayout = new QGridLayout( this, 4, 4, 10 );
	
	topLayout->setRowStretch(0,0);
	topLayout->setRowStretch(1,10);
	topLayout->setRowStretch(2,100);
	topLayout->setRowStretch(3,0);
	
	topLayout->setColStretch(0,0);
	topLayout->setColStretch(1,10);
	topLayout->setColStretch(2,10);
	topLayout->setColStretch(3,0);

	monitorLabel = new QLabel( this );
	monitorLabel->setAlignment( AlignCenter );
	monitorLabel->setPixmap( p );
	monitorLabel->setMinimumSize( 220, 160 );
		 
	topLayout->addMultiCellWidget( monitorLabel, 1, 1, 1, 2 );

	monitor = new KSSMonitor( monitorLabel );
	monitor->setBackgroundColor( black );
	monitor->setGeometry( (monitorLabel->width()-200)/2+20,
		(monitorLabel->height()-160)/2+10, 157, 111 );

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

	QGroupBox *group = new QGroupBox(  i18n("Screen Saver"), this );
	
	topLayout->addWidget( group, 2, 1 );
	
	QBoxLayout *groupLayout = new QVBoxLayout( group, 10, 5 );

	ssList = new QListBox( group );
	ssList->insertItem( i18n("No screensaver"), 0 );
	ssList->setCurrentItem( 0 );
	getSaverNames();
	ssList->adjustSize();
	ssList->setMinimumSize(ssList->size());
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
	
	groupLayout->addSpacing(  20 );		
	groupLayout->addWidget( ssList, 20 );

	setupBt = new QPushButton(  i18n("&Setup ..."), group );
	setupBt->adjustSize();
	setupBt->setFixedHeight( setupBt->height() );
	setupBt->setMinimumWidth(setupBt->width());
	connect( setupBt, SIGNAL( clicked() ), SLOT( slotSetup() ) );
	
	groupLayout->addWidget( setupBt );

	testBt = new QPushButton(  i18n("&Test"), group );
	testBt->adjustSize();
	testBt->setFixedHeight( testBt->height() );
	testBt->setMinimumWidth(testBt->width());
	connect( testBt, SIGNAL( clicked() ), SLOT( slotTest() ) );
	
	groupLayout->addWidget( testBt );
	groupLayout->activate();

	QBoxLayout *stackLayout = new QVBoxLayout( 5 );
	
	topLayout->addLayout( stackLayout, 2, 2 );

	group = new QGroupBox(  i18n("Settings"), this );
	
	stackLayout->addWidget( group, 15 );
	
	groupLayout = new QVBoxLayout( group, 10, 5 );
	
	QBoxLayout *pushLayout = new QHBoxLayout( 5 );
	
	groupLayout->addSpacing( 10 );
	groupLayout->addLayout( pushLayout );
	
	waitEdit = new QLineEdit( group );
	QString str;
	str.setNum( xtimeout/60 );
	waitEdit->setText( str );
	waitEdit->setMaxLength(4);
	waitEdit->adjustSize();
       	connect( waitEdit, SIGNAL( textChanged( const char * ) ),
		 SLOT( slotTimeoutChanged( const char * ) ) );
			
	QLabel *label = new QLabel( waitEdit, i18n("&Wait for"), group );
	label->adjustSize();
	label->setFixedHeight( waitEdit->height() );
	label->setMinimumWidth( label->width() );
	waitEdit->setFixedHeight( waitEdit->height() );
	waitEdit->setMinimumWidth( 50 );

	pushLayout->addWidget( label );		
	pushLayout->addWidget( waitEdit, 10 );
	
	label = new QLabel(  i18n("min."), group );
	label->setFixedHeight( waitEdit->sizeHint().height() );
	label->setMinimumWidth( label->sizeHint().width() );
	
	pushLayout->addWidget( label );

	cb = new QCheckBox(  i18n("&Require password"), group );
	cb->setMinimumSize( cb->sizeHint() );
	cb->setChecked( lock );
	connect( cb, SIGNAL( toggled( bool ) ), SLOT( slotLock( bool ) ) );
	groupLayout->addWidget( cb );

	cbRoot = new QCheckBox( i18n("&Accept root password to unlock"), group );
	cbRoot->setMinimumSize( cbRoot->sizeHint() );
	cbRoot->setChecked( allowRoot );
	connect( cbRoot, SIGNAL( toggled( bool ) ), SLOT( slotAllowRoot( bool ) ) );

	cbRoot->hide(); // Christian: added, because kcheckpass doesn't support this
	//groupLayout->addWidget( cbRoot ); // don't add it to layout, either

	cbStars = new QCheckBox(  i18n("Show &password as stars"), group );
	cbStars->setMinimumSize( cbStars->sizeHint() );
	cbStars->setChecked( showStars );
	connect( cbStars, SIGNAL( toggled( bool ) ), SLOT( slotStars( bool ) ) );
	groupLayout->addWidget( cbStars );

	groupLayout->activate();

	group = new QGroupBox(  i18n("Priority"), this );
	
	stackLayout->addWidget( group, 10 );
	
	groupLayout = new QHBoxLayout( group, 15 );

	prioritySlider = new QSlider( QSlider::Horizontal, group );
	prioritySlider->setRange( 0, 20 );
	prioritySlider->setSteps( 5, 10 );
	prioritySlider->setValue( priority );
	connect( prioritySlider, SIGNAL( valueChanged(int) ),
		SLOT( slotPriorityChanged(int) ) );
		
	label = new QLabel( prioritySlider, i18n("&High"), group );
	
	prioritySlider->setFixedHeight( prioritySlider->sizeHint().height() );
	label->setFixedHeight( prioritySlider->sizeHint().height() );
	label->setMinimumWidth( label->sizeHint().width() );
	
	groupLayout->addWidget( label );
	groupLayout->addWidget( prioritySlider, 10 );
		
	label = new QLabel(  i18n("Low"), group );
	label->setFixedHeight( prioritySlider->sizeHint().height() );
	label->setMinimumWidth( label->sizeHint().width() );
	
	groupLayout->addWidget( label );
	groupLayout->activate();

//	connect( &timer, SIGNAL( timeout() ), SLOT( slotSetupTimeout() ) );

	// I have to call show() here, otherwise the screensaver
	// does not get the correct size information.
	topLayout->activate();
	
	show();

	setMonitor();
}

void KScreenSaver::resizeEvent( QResizeEvent * )
{
	monitor->setGeometry( (monitorLabel->width()-200)/2+20,
		(monitorLabel->height()-160)/2+10, 157, 111 );
}

KScreenSaver::~KScreenSaver()
{
    if(ssPreview && ssPreview->isRunning()) {
       int pid = ssPreview->getPid();  
       ssPreview->kill( );
       waitpid(pid, (int *) 0,0);
    }
    delete ssPreview;
    delete ssSetup;
    delete kssConfig;
}

void KScreenSaver::readSettings( int )
{
	// get X screen saver attribs
	XGetScreenSaver( qt_xdisplay(), &xtimeout, &xinterval,
	       &xprefer_blanking, &xallow_exposures );

	QString str;
	saverLocation = SCREENSAVER_DIR;

//Antonio - Added support to parse the old config file for KDE 1.0 users

        bool first_time = false;
	KConfig *config = kapp->getConfig();
 
	config->setGroup( "ScreenSaver" );
	
	if ( ! config->hasKey("UseSaver") ) {
		first_time = true;
		config= new KConfig(KApplication::kde_configdir() + "/kdisplayrc",
                    KApplication::localconfigdir() + "/kdisplayrc");   
		config->setGroup( "ScreenSaver" );
	};

//Antonio


	saverLocation = config->readEntry( "Location", kapp->kde_bindir().copy() );

	bUseSaver = config->readBoolEntry( "UseSaver", false );
	if( bUseSaver ) {
		str = config->readEntry( "Saver" );
		if ( !str.isNull() )
			saverFile = str;
		else
			bUseSaver = false;
	} else
		saverFile.sprintf( i18n("No screensaver") );

	xtimeout = config->readNumEntry( "Timeout" );
	
	if ( xtimeout == 0 )
		xtimeout = 60;

	str = config->readEntry( "Lock" );
	if ( !str.isNull() && str.find( "yes" ) == 0 )
		lock = TRUE;
	else
		lock = FALSE;

	allowRoot = config->readBoolEntry( "allowRoot", false );

	priority = config->readNumEntry( "Priority", 0 );

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

	showStars = kssConfig->readBoolEntry( "PasswordAsStars", true );

	if (first_time) delete config; //Antonio : Let's delete the config file
}

void KScreenSaver::updateValues()
{
	QString str;
	str.setNum( xtimeout/60 );
	waitEdit->setText( str );

	cb->setChecked( lock );
	cbRoot->setChecked( allowRoot );
	cbStars->setChecked( showStars );

	prioritySlider->setValue( priority );
}

void KScreenSaver::setDefaults()
{
	slotScreenSaver( 0 );
	ssList->setCurrentItem( 0 );
	ssList->centerCurrentItem();
	slotTimeoutChanged( "1" );
	slotPriorityChanged( 0 );
	slotLock( false );
	slotAllowRoot( false );
	slotStars( true );
	slotCornerAction( 0, 'i' );
	slotCornerAction( 1, 'i' );
	slotCornerAction( 2, 'i' );
	slotCornerAction( 3, 'i' );
	updateValues();
}

void KScreenSaver::defaultSettings()
{
	setDefaults();
}

void KScreenSaver::writeSettings()
{
	if ( !changed )
		return;

	KConfig *config = kapp->getConfig();
	config->setGroup( "ScreenSaver" );

	config->writeEntry( "UseSaver", bUseSaver );
	config->writeEntry( "Saver", saverFile );

	QString str;
	str.setNum( xtimeout );
	config->writeEntry( "Timeout", str );

	config->writeEntry( "Lock", lock ? "yes" : "no" );
	config->writeEntry( "allowRoot", allowRoot );

	str.detach();
	str.setNum( priority );
	config->writeEntry( "Priority", str );

	config->writeEntry( "CornerAction", cornerAction );
	
	config->sync();

	kssConfig->writeEntry( "PasswordAsStars", showStars );
	kssConfig->sync();

	changed = FALSE;
}

void KScreenSaver::findSavers()
{
	static QDir d( saverLocation );

	d.setFilter( QDir::Executable | QDir::Files );

	saverList = d.entryList( "*.kss1" );
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
				config->writeEntry( it.current(), buffer, true, false, true );
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

	if ( !bUseSaver )
	{
		// note that changes in the pidFile name have also be done
		// in kdebase/kscreensaver/main.cpp
		QString pidFile;
		pidFile = getenv( "HOME" );
		pidFile += "/.kss-install.pid.";
		char ksshostname[200];
		gethostname(ksshostname, 200);
		pidFile += ksshostname;
		FILE *fp;
		if ( ( fp = fopen( pidFile, "r" ) ) != NULL )
		{
			int pid;
			fscanf( fp, "%d", &pid );
			fclose( fp );
			if( kill( pid, SIGTERM ) == 0)
				waitpid(pid, (int *) 0, 0);
		}
		XSetScreenSaver( qt_xdisplay(), 0, xinterval,
			xprefer_blanking, xallow_exposures );
		return;
	}

	int pid = fork();

	if ( pid == 0 )
	{
		char *lock1, *lock2;

		if( lock ) {
			lock1 = "-lock";
			lock2 = (allowRoot) ? (char *)"-allow-root" : 0;
		} else {
			lock1 = (allowRoot) ? (char *)"-allow-root" : 0;
			lock2 = 0;
		}

		QString sDelay;
		sDelay.setNum( xtimeout/60 );
		QString sPriority;
		sPriority.setNum( priority );
		QString path = saverLocation + '/' + saverFile;

		execl( path.data(), path.data(), "-delay", sDelay.data(), "-install",
				"-corners", cornerAction, "-nice", sPriority.data(),
				lock1, lock2, 0 );
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

    if ( !bUseSaver )
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
		saverFile.detach();
		saverFile.sprintf( i18n("No screensaver") );
		setupBt->setEnabled( FALSE );
		testBt->setEnabled( FALSE );
		bUseSaver = false;
	}
	else
	{
		QStrListIterator it( *saverList );
		saverFile = it += (indx - 1);
		if (!ssSetup->isRunning())
			setupBt->setEnabled( TRUE );
		testBt->setEnabled( TRUE );
		bUseSaver = true;
	}

	setMonitor();

	changed = TRUE;
}

void KScreenSaver::slotSetup()
{
	QString path;

	if ( !bUseSaver )
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

void KScreenSaver::slotAllowRoot( bool a )
{
	allowRoot = a;
	changed = TRUE;
}

void KScreenSaver::slotStars( bool s )
{
	showStars = s;
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
	setMonitor();
	setupBt->setEnabled( TRUE );
}

void KScreenSaver::slotHelp()
{
	kapp->invokeHTMLHelp( "kcmdisplay/kdisplay-4.html", "" );
}

void KScreenSaver::slotCornerAction( int num, char action )
{
	cornerAction[num] = action;
	changed = TRUE;
}

void KScreenSaver::loadSettings()
{
  ssList->clear();  
  ssList->insertItem( i18n("No screensaver"), 0 );

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

  updateValues();
}

void KScreenSaver::applySettings()
{
  writeSettings();
  apply(TRUE);
}
