//-----------------------------------------------------------------------------
//
// KDE Display color scheme setup module
//
// Copyright (c)  Mark Donohoe 1997
//

/*
 *
 *    $Id$
 *
 */

#include <stdio.h>
#include <stdlib.h>
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
#include <qcombo.h>
#include <qlayout.h>
#include <kapp.h>
#include <kcharsets.h>
#include <kconfigbase.h>
#include <ksimpleconfig.h>

#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>


#include "kcolordlg.h"
#include "kfontdialog.h"

#include "general.h"
#include "general.moc"

extern int dropError(Display *, XErrorEvent *);


FontUseItem::FontUseItem( const char *n, QFont default_fnt, bool f = false )
	: selected(0)
{
	_text = n;
	_font = default_fnt;
	fixed = f;
}

void FontUseItem::setRC( const char *group, const char *key, const char *rc )
{
	_rcgroup = group;
	_rckey = key;
	if ( rc ) _rcfile = rc;
}

void FontUseItem::readFont()
{
	KConfigBase *config;
	if ( _rcfile.isEmpty() ) {
		config  = kapp->getConfig();
	} else {
		QString s( KApplication::localconfigdir() );
		s += "/";
		s += _rcfile;
		config = new KSimpleConfig( s.data(), true );
	}
	
	config->setGroup( _rcgroup.data() );
	_font = config->readFontEntry( _rckey.data(), new QFont( _font ) );
	debug("Reading font %s", _rckey.data() );
}

void FontUseItem::writeFont()
{
	KConfigBase *config;
	if ( _rcfile.isEmpty() ) {
		config  = kapp->getConfig();
	} else {
		QString s( KApplication::localconfigdir() );
		s += "/";
		s += _rcfile;
		debug("Open resource file : %s", s.data() );
		config = new KSimpleConfig( s.data() );
	}
	
	config->setGroup( _rcgroup.data() );
	if ( _rcfile.isEmpty() )
		 config->writeEntry( _rckey.data(), _font, true, true );
	else {
		config->writeEntry( _rckey.data(), _font );
		config->sync();
	}
	debug("Writing font %s", _rckey.data() );
}

KFontChooser::KFontChooser( QWidget *parent, const char *name )
	: QWidget( parent, name )
{
	int i;
	
	fnt = QFont( "helvetica", 12 );
	changed = False;
	
	QBoxLayout *topLayout = new QVBoxLayout( this, 10 );
	topLayout->addStretch( 5 );
	
	QBoxLayout *stackLayout = new QVBoxLayout( 4 );
	
	topLayout->addLayout( stackLayout );
		
	cmbFont = new QComboBox( false, this );
	cmbFont->setFixedHeight( cmbFont->sizeHint().height() );
	
	getFontList( fixedList, true );
	getFontList( fontList );
	
	cmbFont->insertStrList( &fontList );
	QStrListIterator it( fontList );
	for ( i = 0; it.current(); ++it, i++ ) {
		if ( !strcmp( fnt.family(), it.current() ) )
			cmbFont->setCurrentItem( i );
	}
	
	connect( cmbFont, SIGNAL( activated( const char * ) ),
		SLOT( slotSelectFont( const char * ) ) );
		
	QLabel *label = new QLabel( cmbFont, "&Typeface", this );
	label->setFixedHeight( label->sizeHint().height() );
	
	stackLayout->addWidget( label );
	stackLayout->addWidget( cmbFont );

	cbBold = new QCheckBox(  i18n("&Bold"), this );
	cbBold->setMinimumSize( cbBold->sizeHint() );
	cbBold->setChecked( fnt.bold() );
	connect( cbBold, SIGNAL( toggled( bool ) ), SLOT( slotFontBold( bool ) ) );
	
	topLayout->addWidget( cbBold );
	
	cbItalic = new QCheckBox(  i18n("&Italic"), this );
	cbItalic->setMinimumSize( cbItalic->sizeHint() );
	cbItalic->setChecked( fnt.italic() );
	connect( cbItalic, SIGNAL( toggled( bool ) ), SLOT( slotFontItalic( bool ) ) );
	
	topLayout->addWidget( cbItalic );
	
	QBoxLayout *pushLayout = new QHBoxLayout(  2 );
	
	topLayout->addLayout( pushLayout );
	
	stackLayout = new QVBoxLayout( 4 );
	
	pushLayout->addLayout( stackLayout, 10 );
	pushLayout->addSpacing( 10 );
	
	sbSize = new KNumericSpinBox( this );
	
	sbSize->setStep( 1 );
	sbSize->setRange( 8, 18 );
	sbSize->setValue( 12 );
	
	connect( sbSize, SIGNAL( valueDecreased() ),
		 SLOT( slotFontSize() ) );
		 
	connect( sbSize, SIGNAL( valueIncreased() ),
		 SLOT( slotFontSize() ) );
	
	label = new QLabel( sbSize, "&Size", this );
	label->setFixedHeight( label->sizeHint().height() );

	cmbCharset = new QComboBox( false, this );
	
	fillCharsetCombo();
	cmbCharset->setInsertionPolicy( QComboBox::NoInsertion );
	connect( cmbCharset, SIGNAL( activated( const char * ) ),
		 SLOT( slotCharset( const char * ) ) );
		 
	sbSize->setFixedHeight( cmbCharset->sizeHint().height() );
	cmbCharset->setFixedHeight( cmbCharset->sizeHint().height() );
	
	stackLayout->addWidget( label );
	stackLayout->addWidget( sbSize );
	
	stackLayout = new QVBoxLayout( 4 );
	
	pushLayout->addLayout( stackLayout, 30 );
	
	label = new QLabel( cmbCharset, "&Character set", this );
	label->setFixedHeight( label->sizeHint().height() );
	
	stackLayout->addWidget( label );
	stackLayout->addWidget( cmbCharset );

	topLayout->activate();
}

void KFontChooser::setFont( QFont start_fnt, bool fixed )
{
	fnt = start_fnt;
	
	cmbFont->clear();
	if( fixed )
		cmbFont->insertStrList( &fixedList );
	else 
		cmbFont->insertStrList( &fontList );
	
	QStrListIterator it( fixed ? fixedList : fontList );
	for ( int i = 0; it.current(); ++it, i++ ) {
		if ( !strcmp( fnt.family(), it.current() ) )
			cmbFont->setCurrentItem( i );
	}
	
	sbSize->setValue( fnt.pointSize() );
	
	if ( fnt.bold() )
		cbBold->setChecked( true );
	else
		cbBold->setChecked( false );
		
	if ( fnt.italic() )
		cbItalic->setChecked( true );
	else
		cbItalic->setChecked( false );
}

void KFontChooser::getFontList( QStrList &list, const char *pattern )
{
	int num;
	char **xFonts = XListFonts( qt_xdisplay(), pattern, 1000, &num );

	debug("number of fonts found %d", num);

	for ( int i = 0; i < num; i++ )
	{
		addFont( list, xFonts[i] );
	}

	XFreeFontNames( xFonts );
}


void KFontChooser::getFontList( QStrList &list, bool fixed )
{
	// Use KDE fonts fs there is a KDE font list and only if the KDE fonts
	// exist on the server where the desktop is running.
	
	QStrList lstSys, lstKDE;
	
	if ( fixed ) {
		getFontList( lstSys, "-*-*-*-*-*-*-*-*-*-*-m-*-*-*" );
		getFontList( lstSys, "-*-*-*-*-*-*-*-*-*-*-c-*-*-*" );
	} else
		getFontList( lstSys, "-*-*-*-*-*-*-*-*-*-*-*-*-*-*" );
		
	if ( !kapp->getKDEFonts( &lstKDE ) ) {
		debug("No kapp fonts found");
		list = lstSys;
		return;
	}
	
	for( int i = 0; i < lstKDE.count(); i++ ) {
		if ( lstSys.find( lstKDE.at( i ) ) != -1 ) {
			list.append( lstKDE.at( i ) );
		}
	}
}

void KFontChooser::addFont( QStrList &list, const char *xfont )
{
	const char *ptr = strchr( xfont, '-' );
	if ( !ptr )
		return;
	
	ptr = strchr( ptr + 1, '-' );
	if ( !ptr )
		return;

	QString font = ptr + 1;

	int pos;
	if ( ( pos = font.find( '-' ) ) > 0 )
	{
		font.truncate( pos );

		if ( font.find( "open look", 0, false ) >= 0 )
			return;

		QStrListIterator it( list );

		for ( ; it.current(); ++it )
			if ( it.current() == font )
				return;

		list.inSort( font );
	}
}

void KFontChooser::fillCharsetCombo(){
int i;
	cmbCharset->clear();
	KCharsets *charsets=kapp->getCharsets();
        QStrList sets=charsets->displayable(fnt.family());
	cmbCharset->insertItem( i18n("default") );
	for(QString set=sets.first();set;set=sets.next())
	  cmbCharset->insertItem( set );
	cmbCharset->insertItem( i18n("any") );

	QString charset=charsets->name(fnt);
	for(i = 0;i<cmbCharset->count();i++){
	  if (charset==cmbCharset->text(i)){
	    cmbCharset->setCurrentItem(i);
	    break;
	  }
	}
}

void KFontChooser::slotCharset(const char *name)
{

  KCharsets *charsets=kapp->getCharsets();
  if (strcmp(name,"default")==0){
     charsets->setQFont(fnt,klocale->charset());
     defaultCharset=TRUE;
  }   
  else{   
     charsets->setQFont(fnt,name);
     defaultCharset=FALSE;
  }   

  changed=TRUE;
}

void KFontChooser::slotSelectFont( const char *fname )
{
//	if( lbFonts->currentItem() == 0 )
	fnt.setFamily( fname );
		
	//fillCharsetCombo();	
	//slotPreviewFont(0);
	emit fontChanged( fnt );
	changed=TRUE;
}

void KFontChooser::slotFontSize( )
{
	//const int sizes[] = { 10, 12, 14 };

//	if( lbFonts->currentItem() == 0 )
	int s = sbSize->getValue();
		fnt.setPointSize( s );
	
	//slotPreviewFont(0);
	emit fontChanged( fnt );

	changed=TRUE;
}

void KFontChooser::slotFontBold( bool b )
{
//	if( lbFonts->currentItem() == 0 )
		fnt.setBold( b );
	
	//slotPreviewFont(0);
	emit fontChanged( fnt );

	changed=TRUE;
}

void KFontChooser::slotFontItalic( bool i )
{
//	if( lbFonts->currentItem() == 0 )
		fnt.setItalic( i );
	
	//slotPreviewFont(0);
	emit fontChanged( fnt );

	changed=TRUE;
}


//------------------------------------------------------------------

KGeneral::KGeneral( QWidget *parent, int mode, int desktop )
	: KDisplayModule( parent, mode, desktop )
{	
        int i;
	changed = FALSE;
	
	// if we are just initialising we don't need to create setup widget
	if ( mode == Init )
		return;
	
	kde_display = x11Display();
	KDEChangeGeneral = XInternAtom( kde_display, "KDEChangeGeneral", False);
	screen = DefaultScreen(kde_display);
	root = RootWindow(kde_display, screen);

	setName( i18n("Fonts etc.") );

	readSettings();
	
	QBoxLayout *topLayout = new QVBoxLayout( this, 10 );
	
	cbStyle = new QCheckBox( 
	i18n( "&Draw widgets in the style of Windows 95" ), this );
	
	if( applicationStyle == WindowsStyle )
		cbStyle->setChecked( True );
	else
		cbStyle->setChecked( False);
	
	connect( cbStyle, SIGNAL( clicked() ), SLOT( slotChangeStyle()  )  );
	
	topLayout->addWidget( cbStyle, 10 );
	
	stCombo = new QComboBox( this );
	stCombo->hide();
	//stCombo->setGeometry( 15, 35, 120, 25 );
	//stCombo->insertItem( i18n("Motif") );
	//stCombo->insertItem( i18n("Windows 95") );
	//connect( stCombo, SIGNAL( activated(int) ),
	//		SLOT( slotChangeStyle(int)  )  );
			
	//if(applicationStyle==WindowsStyle)
	//	stCombo->setCurrentItem(1);
	//else
	//	stCombo->setCurrentItem(0);

	QGroupBox *group = new QGroupBox( i18n( "Desktop fonts" ), this );
	
	topLayout->addWidget( group, 50 );
	
	QBoxLayout *groupLayout = new QVBoxLayout( group, 10 );
	
	groupLayout->addSpacing( 20 );
	
	QBoxLayout *pushLayout = new QHBoxLayout( 5 );
	
	groupLayout->addLayout( pushLayout );
	
	/*groupLayout->setRowStretch(0,0);
	groupLayout->setRowStretch(1,10);
	groupLayout->setRowStretch(2,10);
	groupLayout->setRowStretch(3,0);
	
	groupLayout->setColStretch(0,0);
	groupLayout->setColStretch(1,10);
	groupLayout->setColStretch(2,10);
	groupLayout->setColStretch(3,0);*/
	
	FontUseItem *item = new FontUseItem( i18n("General font"),
				QFont( "helvetica", 12 ) );
	item->setRC( "Desktop Fonts", "GeneralFont" );
	fontUseList.append( item );
	
	item = new FontUseItem( i18n("Fixed font"),
				QFont( "fixed", 10 ), true );
	item->setRC( "Desktop Fonts", "FixedFont" );
	fontUseList.append( item );
	
	item = new FontUseItem( i18n("Window title font"),
				QFont( "helvetica", 12, QFont::Bold ) );
	item->setRC( "Desktop Fonts", "WindowTitleFont" );
	fontUseList.append( item );
				
	item = new FontUseItem( i18n("Panel button font"),
				QFont( "helvetica", 12 )  );
	item->setRC( "Fonts", "DesktopButtonFont", "kpanelrc" );
	fontUseList.append( item );
	
	item = new FontUseItem( i18n("Panel clock font"),
				QFont( "times", 14, QFont::Normal, true ) );
	item->setRC( "Fonts", "DateFont", "kpanelrc" );
	fontUseList.append( item );
	
	for ( i=0; i<fontUseList.count(); i++ )
		fontUseList.at( i )->readFont();
	
	lbFonts = new QListBox( group );
	for ( i=0; i<fontUseList.count(); i++ )
		lbFonts->insertItem( fontUseList.at( i )->text() );
	
	/*lbFonts->insertItem( i18n("General font") );
	lbFonts->insertItem( i18n("Fixed font") );
	lbFonts->insertItem( i18n("Window title font") );
	lbFonts->insertItem( i18n("Panel button font") );
	lbFonts->insertItem( i18n("Panel clock font") );*/
	
	lbFonts->setCurrentItem( 0 );
	connect( lbFonts, SIGNAL( highlighted( int ) ),
			SLOT( slotPreviewFont( int ) ) );
			
	pushLayout->addWidget( lbFonts );
	
	fntChooser = new KFontChooser( group );
	
	connect( fntChooser, SIGNAL( fontChanged( QFont ) ), this,
		SLOT( slotSetFont( QFont ) ) );
	
	pushLayout->addWidget( fntChooser );
	
	QBoxLayout *stackLayout = new QVBoxLayout( 4 );
	
	groupLayout->addLayout( stackLayout );
	
	QLabel *label = new QLabel( "Sample text", group );
	label->setFixedHeight( label->sizeHint().height() );
	
	stackLayout->addWidget( label );
	
	lSample = new QLabel( group );
	lSample->setText( "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG\n"\
						"the quick brown fox jumps over the lazy dog\n"\
						"0 1 2 3 4 5 6 7 8 9   ! \" £ $ % ^ & * ( )" );
	lSample->setAlignment( AlignLeft | AlignVCenter );
	lSample->setFixedHeight( 2*lSample->sizeHint().height() );
	lSample->setFrameStyle( QFrame::Panel | QFrame::Sunken );
	
	stackLayout->addWidget( lSample );

	topLayout->activate();
	//connectColor();
	//setColors();
}


void KGeneral::slotChangeStyle()
{
	if( cbStyle->isChecked() )
		applicationStyle = WindowsStyle;
	else
		applicationStyle = MotifStyle;
				
	changed=TRUE;
}

KGeneral::~KGeneral()
{
}



void KGeneral::readSettings( int )
{		
	QString str;

	KConfig config;
	config.setGroup( "Desktop General Settings" );

	str = config.readEntry( "WidgetStyle" );
	if ( !str.isNull() ) {
		if ( str == "Windows 95" )
			applicationStyle = WindowsStyle;
		else
			applicationStyle = MotifStyle;
	} else
		applicationStyle = MotifStyle;
}

void KGeneral::writeSettings()
{
	if ( !changed )
		return;
		
	for ( int i=0; i<fontUseList.count(); i++ )
		fontUseList.at( i )->writeFont();	
	
	debug("Written fonts");
	
	KConfig *config = kapp->getConfig();
	config->setGroup( "Desktop General Settings" );

	QString str;
	if( applicationStyle == WindowsStyle )
		str.sprintf("Windows 95" );
	else
		str.sprintf("Motif" );
	config->writeEntry("WidgetStyle", str, true, true);
	config->sync();
}

void KGeneral::slotApply()
{
	writeSettings();
	apply();
}

//Matthias
static int _getprop(Window w, Atom a, Atom type, long len, unsigned char **p){
  Atom real_type;
  int format;
  unsigned long n, extra;
  int status;
  
  status = XGetWindowProperty(qt_xdisplay(), w, a, 0L, len, False, type, &real_type, &format, &n, &extra, p);
  if (status != Success || *p == 0)
    return -1;
  if (n == 0)
    XFree((void*) *p);
  return n;
}

//Matthias
static bool getSimpleProperty(Window w, Atom a, long &result){
  long *p = 0;
  
  if (_getprop(w, a, a, 1L, (unsigned char**)&p) <= 0){
    return FALSE;
  }
  
  result = p[0];
  XFree((char *) p);
  return TRUE;
}

void KGeneral::apply( bool  )
{
	if ( !changed )
		return;
	
	XEvent ev;
	unsigned int i, nrootwins;
	Window dw1, dw2, *rootwins;
	int (*defaultHandler)(Display *, XErrorEvent *);


	defaultHandler=XSetErrorHandler(dropError);
	
	XQueryTree(kde_display, root, &dw1, &dw2, &rootwins, &nrootwins);
	
	// Matthias
	Atom a = XInternAtom(qt_xdisplay(), "KDE_DESKTOP_WINDOW", False);
	for (i = 0; i < nrootwins; i++) {
	  long result = 0;
	  getSimpleProperty(rootwins[i],a, result);
	  if (result){
	    ev.xclient.type = ClientMessage;
	    ev.xclient.display = kde_display;
	    ev.xclient.window = rootwins[i];
	    ev.xclient.message_type = KDEChangeGeneral;
	    ev.xclient.format = 32;
	    
	    XSendEvent(kde_display, rootwins[i] , False, 0L, &ev);
	  }
	}

	XFlush(kde_display);
	XSetErrorHandler(defaultHandler);
	
	XFree((void *) rootwins);
	
	changed=FALSE;
}

void KGeneral::slotSetFont( QFont fnt )
{
	fontUseList.current()->setFont( fnt );
	lSample->setFont( fnt );
	changed = true;
}


void KGeneral::slotPreviewFont( int index )
{
	fntChooser->setFont( fontUseList.at( index )->font(),  
			fontUseList.at( index )->spacing() );
	lSample->setFont( fontUseList.at( index )->font() );
}

void KGeneral::slotHelp()
{
	kapp->invokeHTMLHelp( "kdisplay/kdisplay-6.html", "" );
}


void KGeneral::loadSettings()
{
  int i = 0;

  readSettings();

	if(applicationStyle==WindowsStyle)
		stCombo->setCurrentItem(1);
	else
		stCombo->setCurrentItem(0);

	lbFonts->setCurrentItem( 0 );

	/*QStrListIterator it( fontList );
	for (i = 0; it.current(); ++it, i++ )
	{
		if ( !strcmp( generalFont.family(), it.current() ) )
			cmbFont->setCurrentItem( i );
	}
	example_label->setFont(generalFont);

	if ( generalFont.pointSize() <= 10 )
		((QRadioButton *)btnGrp->find( 0 ))->setChecked( TRUE );
	else if ( generalFont.pointSize() <= 12 )
		((QRadioButton *)btnGrp->find( 1 ))->setChecked( TRUE );
	else
		((QRadioButton *)btnGrp->find( 2 ))->setChecked( TRUE );

	cbBold->setChecked( generalFont.bold() );
	
	cbItalic->setChecked( generalFont.italic() );

	fillCharsetCombo();*/
}

void KGeneral::applySettings()
{
  writeSettings();
  apply( TRUE );
}


void KGeneral::connectColor(){

  connect(KApplication::getKApplication(),SIGNAL(kdisplayPaletteChanged()),
	  this,SLOT(setColors())); 
}

void KGeneral::setColors(){
 
  /* this is to the the backgound of a widget to white and the
     text color to black -- some lables such as the one of the
     font manager really shouldn't follow colorschemes The
     primary task of those label is to display the text clearly
     an visibly and not to look cool ...*/

  QPalette mypalette = (example_label->palette()).copy();

  QColorGroup cgrp = mypalette.normal();
  QColorGroup ncgrp(black,cgrp.background(),
		    cgrp.light(),cgrp.dark(),cgrp.mid(),black,white);

  mypalette.setNormal(ncgrp);
  mypalette.setDisabled(ncgrp);
  mypalette.setActive(ncgrp);

  example_label->setPalette(mypalette);
  example_label->setBackgroundColor(white);


}
