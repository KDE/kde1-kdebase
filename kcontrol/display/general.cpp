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
#include "fontchooser.h"
#include "kresourceman.h"

#include "general.h"
#include "general.moc"

extern int dropError(Display *, XErrorEvent *);

FontUseItem::FontUseItem( const char *n, QFont default_fnt, bool f = false )
	: selected(0)
{
	_text = n;
	_default = _font = default_fnt;
	fixed = f;
}

void FontUseItem::setRC( const char *group, const char *key, const char *rc )
{
	_rcgroup = group;
	_rckey = key;
	if ( rc ) _rcfile = rc;
}

void FontUseItem::setDefault()
{
	_font = _default;
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
		config = new KSimpleConfig( s.data() );
	}
	
	config->setGroup( _rcgroup.data() );
	if ( _rcfile.isEmpty() )
		 config->writeEntry( _rckey.data(), _font, true, true );
	else {
		config->writeEntry( _rckey.data(), _font );
		config->sync();
	}
	
	//delete config;
}

//------------------------------------------------------------------

void FontPreview::fontChange( const QFont & )
{
	if ( !(_fnt == font() ) ) {
		QLabel::setFont( _fnt );
	}
	update();
}

void FontPreview::setPreviewFont( const QFont &fnt )
{
	_fnt = fnt;
	setFont( fnt );
}


//------------------------------------------------------------------

KGeneral::KGeneral( QWidget *parent, int mode, int desktop )
	: KDisplayModule( parent, mode, desktop )
{
	int i;
	changed = false;
	useRM = false;
	
	debug("KGeneral::KGeneral");
	
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
	
	cbStyle = new QCheckBox( i18n( "&Draw widgets in the style of Windows 95" ), 
				 this );
	cbStyle->adjustSize();
	cbStyle->setMinimumSize(cbStyle->size());

	if( applicationStyle == WindowsStyle )
	        cbStyle->setChecked( true );
	else
		cbStyle->setChecked( false);
	
	connect( cbStyle, SIGNAL( clicked() ), SLOT( slotChangeStyle()  )  );
	
	topLayout->addWidget( cbStyle, 10 );

	QGroupBox *group = new QGroupBox( i18n( "Desktop fonts" ), this );
	
	topLayout->addWidget( group, 100 );
	
	QBoxLayout *groupLayout = new QVBoxLayout( group, 10, 5 );
	
	groupLayout->addSpacing( 15 );
	
	QBoxLayout *pushLayout = new QHBoxLayout( 5 );
	
	groupLayout->addLayout( pushLayout );
	
	FontUseItem *item = new FontUseItem( i18n("General font"),
				QFont( "helvetica", 12 ) );
	item->setRC( "General", "font" );
	fontUseList.append( item );
	
	item = new FontUseItem( i18n("Fixed font"),
				QFont( "fixed", 10 ), true );
	item->setRC( "General", "fixedFont" );
	fontUseList.append( item );
	
	item = new FontUseItem( i18n("Window title font"),
				QFont( "helvetica", 12, QFont::Bold ) );
	item->setRC( "WM", "titleFont" );
	fontUseList.append( item );
				
	item = new FontUseItem( i18n("Panel button font"),
				QFont( "helvetica", 12 )  );
	item->setRC( "Fonts", "DesktopButtonFont", "kpanelrc" );
	fontUseList.append( item );
	
	item = new FontUseItem( i18n("Panel clock font"),
				QFont( "times", 14, QFont::Normal, true ) );
	item->setRC( "Fonts", "DateFont", "kpanelrc" );
	fontUseList.append( item );
	
	for ( i = 0; i < (int) fontUseList.count(); i++ )
		fontUseList.at( i )->readFont();
	
	lbFonts = new QListBox( group );
	for ( i=0; i < (int) fontUseList.count(); i++ )
  	     lbFonts->insertItem( fontUseList.at( i )->text() );
	lbFonts->adjustSize();
	lbFonts->setMinimumSize(lbFonts->size());
	
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
	label->adjustSize();
	label->setFixedHeight( label->height() );
	label->setMinimumWidth(label->width());

	stackLayout->addWidget( label );
	
	lSample = new FontPreview( group );
	lSample->setText( "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG\n"
			  "the quick brown fox jumps over the lazy dog\n"
			  "0 1 2 3 4 5 6 7 8 9   ! \" £ $ % ^ & * ( )" );
	lSample->setAlignment( AlignLeft | AlignVCenter );
	lSample->setFixedHeight( 2*lSample->sizeHint().height() );
	lSample->setFrameStyle( QFrame::Panel | QFrame::Sunken );
	lSample->adjustSize();
	lSample->setMinimumSize(lSample->size());

	stackLayout->addWidget( lSample );
	groupLayout->activate();
		
	lbFonts->setCurrentItem( 0 );

	topLayout->activate();
}


void KGeneral::slotChangeStyle()
{
	if( cbStyle->isChecked() )
		applicationStyle = WindowsStyle;
	else
		applicationStyle = MotifStyle;
				
	changed=true;
}

KGeneral::~KGeneral()
{
}



void KGeneral::readSettings( int )
{		
	QString str;

	KConfig config;
	config.setGroup( "KDE" );

	str = config.readEntry( "widgetStyle", "Motif" );
	if ( str == "Windows 95" )
		applicationStyle = WindowsStyle;
	else
		applicationStyle = MotifStyle;

		
	KSimpleConfig appConfig( KApplication::localconfigdir() + "/kdisplayrc" );
	useRM = appConfig.readBoolEntry( "useResourceManager", false );
}

void KGeneral::setDefaults()
{
	int ci = lbFonts->currentItem();
	for ( int i = 0; i < (int) fontUseList.count(); i++ )
		fontUseList.at( i )->setDefault();
	fontUseList.at( ci );
	slotPreviewFont( ci );
	
	cbStyle->setChecked( false );
	slotChangeStyle();
}

void KGeneral::writeSettings()
{
	if ( !changed )
		return;
		
	for ( int i = 0; i < (int) fontUseList.count(); i++ )
		fontUseList.at( i )->writeFont();	
	
	KConfig *config = kapp->getConfig();
	config->setGroup( "KDE" );

	QString str;
	if( applicationStyle == WindowsStyle )
		str.sprintf("Windows 95" );
	else
		str.sprintf("Motif" );
	config->writeEntry("widgetStyle", str, true, true);
	config->sync();
	
	if ( useRM ) {
		QApplication::setOverrideCursor( waitCursor );

		KResourceMan *krdb = new KResourceMan();
		for ( int i = 0; i < (int) fontUseList.count(); i++ ) {
			FontUseItem *it = fontUseList.at( i );
			if ( !it->rcFile() ) {
				krdb->setGroup( it->rcGroup() );
				krdb->writeEntry( it->rcKey(), it->font() );
			}
		}
		krdb->setGroup( "KDE" );
		krdb->writeEntry( "widgetStyle", str );
		krdb->sync();

		QApplication::restoreOverrideCursor();
	}
	
	fontUseList.at( lbFonts->currentItem() );
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
    return false;
  }
  
  result = p[0];
  XFree((char *) p);
  return true;
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
	
	changed=false;
}

void KGeneral::slotSetFont( QFont fnt )
{
	fontUseList.current()->setFont( fnt );
	lSample->setPreviewFont( fnt );
	changed = true;
}

void KGeneral::slotPreviewFont( int index )
{
	fntChooser->setFont( fontUseList.at( index )->font(),  
			fontUseList.at( index )->spacing() );
	lSample->setPreviewFont( fontUseList.at( index )->font() );
}

void KGeneral::slotHelp()
{
	kapp->invokeHTMLHelp( "kdisplay/kdisplay-6.html", "" );
}

void KGeneral::applySettings()
{
  writeSettings();
  apply( true );
}





