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
#include <kapp.h>
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

#define CHARSETS_COUNT 9
static char *charsetsStr[CHARSETS_COUNT]={"ISO-8859-1","Any",
                                   "ISO-8859-2",
                                   "ISO-8859-3",
                                   "ISO-8859-4",
                                   "ISO-8859-5",
                                   "ISO-8859-6",
                                   "ISO-8859-7",
                                   "ISO-8859-8"};

static QFont::CharSet charsetsIds[CHARSETS_COUNT]={ QFont::ISO_8859_1,
                                             QFont::AnyCharSet,
					     QFont::ISO_8859_2,
					     QFont::ISO_8859_3,
					     QFont::ISO_8859_4,
					     QFont::ISO_8859_5,
					     QFont::ISO_8859_6,
					     QFont::ISO_8859_7,
					     QFont::ISO_8859_8};

extern int dropError(Display *, XErrorEvent *);

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

	setName( klocale->translate("General") );

	readSettings();
	
	QGroupBox *group = new QGroupBox( klocale->translate("Widget style"), this );
	group->setGeometry( 15, 85, 155, 80 );

	stCombo = new QComboBox( group );
	stCombo->setGeometry( 15, 35, 120, 25 );
	stCombo->insertItem( klocale->translate("Motif") );
	stCombo->insertItem( klocale->translate("Windows 95") );
	connect( stCombo, SIGNAL( activated(int) ),
			SLOT( slotChangeStyle(int)  )  );
			
	if(applicationStyle==WindowsStyle)
		stCombo->setCurrentItem(1);
	else
		stCombo->setCurrentItem(0);

	group = new QGroupBox( klocale->translate("Fonts"), this );
	group->setGeometry( 15, 190, 440, 170 );
	
	fontTypeList = new QListBox( group );
	fontTypeList->setGeometry( 15, 20, 140, 135 );
	fontTypeList->insertItem( klocale->translate("General font") );
	fontTypeList->setCurrentItem( 0 );
	connect( fontTypeList, SIGNAL( highlighted( int ) ),
			SLOT( slotPreviewFont( int ) ) );

	fontCombo = new QComboBox( false, group );
	fontCombo->setGeometry( 175, 20, 150, 30 );
	getFontList( fontList, "-*-*-*-*-*-*-*-*-*-*-p-*-*-*" );
	fontCombo->insertStrList( &fontList );
	QStrListIterator it( fontList );
	for ( i = 0; it.current(); ++it, i++ )
	{
		if ( !strcmp( generalFont.family(), it.current() ) )
			fontCombo->setCurrentItem( i );
	}
	connect( fontCombo, SIGNAL( activated( const char * ) ),
		SLOT( slotSelectFont( const char * ) ) );
			
	example_label = new QLabel( group );
	example_label->setFont(generalFont);
	example_label->setGeometry( 345, 20, 85, 30 );
	example_label->setAlignment(AlignCenter);
	//	example_label->setBackgroundColor(white);
	example_label->setFrameStyle( QFrame::WinPanel | QFrame::Sunken );
	example_label->setLineWidth( 1 );
	example_label->setText(klocale->translate("Sample"));

	btnGrp = new QButtonGroup( group );
	btnGrp->hide();
	btnGrp->setExclusive( true );

	QRadioButton *rb = new QRadioButton( klocale->translate("Small"), group );
	rb->setGeometry( 175, 60, 90, 25 );
	btnGrp->insert( rb );

	rb = new QRadioButton( klocale->translate("Medium"), group );
	rb->setGeometry( 260, 60, 90, 25 );
	btnGrp->insert( rb );

	rb = new QRadioButton( klocale->translate("Large"), group );
	rb->setGeometry( 350, 60, 85, 25 );
	btnGrp->insert( rb );

	if ( generalFont.pointSize() <= 10 )
		((QRadioButton *)btnGrp->find( 0 ))->setChecked( TRUE );
	else if ( generalFont.pointSize() <= 12 )
		((QRadioButton *)btnGrp->find( 1 ))->setChecked( TRUE );
	else
		((QRadioButton *)btnGrp->find( 2 ))->setChecked( TRUE );
	connect( btnGrp, SIGNAL( clicked( int ) ), SLOT( slotFontSize( int ) ) );

	cb1 = new QCheckBox(  klocale->translate("Bold"), group );
	cb1->setGeometry( 175, 90, 90, 25 );
	cb1->setChecked( generalFont.bold() );
	connect( cb1, SIGNAL( toggled( bool ) ), SLOT( slotFontBold( bool ) ) );
	
	cb2 = new QCheckBox(  klocale->translate("Italic"), group );
	cb2->setGeometry( 260, 90, 90, 25 );
	cb2->setChecked( generalFont.italic() );
	connect( cb2, SIGNAL( toggled( bool ) ), SLOT( slotFontItalic( bool ) ) );
	
	QPushButton *button = new QPushButton(  klocale->translate("Help"), group );
	button->setGeometry( 360, 90, 70, 25 );
	connect( button, SIGNAL( clicked() ), SLOT( slotHelp() ) );

	charset_label = new QLabel(group,"charset");
	charset_label->setText(klocale->translate("Charset:"));
	charset_label->setGeometry(180,125, 70, 25);

	charset_combo = new QComboBox( false, group );
	charset_combo->setGeometry(250,125, 180, 30);

	for(i=0;i<CHARSETS_COUNT;i++)
	  charset_combo->insertItem( charsetsStr[i] );

	charset_combo->setInsertionPolicy(QComboBox::NoInsertion);
	connect( charset_combo, SIGNAL(activated(int)),
		 SLOT(slotCharset(int)) );

	QFont::CharSet charset=generalFont.charSet();

	for(i = 0;i<CHARSETS_COUNT;i++){
	  if (charset==charsetsIds[i]){
	    charset_combo->setCurrentItem(i);
	    break;
	  }
	}

	connectColor();
	setColors();

}


void KGeneral::slotCharset(int index)
{

  generalFont.setCharSet(charsetsIds[index]);

  changed=TRUE;

}

void KGeneral::slotSelectFont( const char *fname )
{
	if( fontTypeList->currentItem() == 0 )
		generalFont.setFamily( fname );
	
	slotPreviewFont(0);

	changed=TRUE;
}

void KGeneral::slotFontSize( int s )
{
	const int sizes[] = { 10, 12, 14 };

	if( fontTypeList->currentItem() == 0 )
		generalFont.setPointSize( sizes[s] );
	
	slotPreviewFont(0);

	changed=TRUE;
}

void KGeneral::slotFontBold( bool b )
{
	if( fontTypeList->currentItem() == 0 )
		generalFont.setBold( b );
	
	slotPreviewFont(0);

	changed=TRUE;
}

void KGeneral::slotFontItalic( bool i )
{
	if( fontTypeList->currentItem() == 0 )
		generalFont.setItalic( i );
	
	slotPreviewFont(0);

	changed=TRUE;
}

void KGeneral::slotChangeStyle(int)
{
	int selection;
	
	selection = stCombo->currentItem()+1;
	switch(selection) {
		case 1:	applicationStyle=MotifStyle;
				break;
		case 2:	applicationStyle=WindowsStyle;
				break;
	}
	
	changed=TRUE;
}

KGeneral::~KGeneral()
{
}

void KGeneral::getFontList( QStrList &list, const char *pattern )
{
	int num;

	char **xFonts = XListFonts( qt_xdisplay(), pattern, 200, &num );

	for ( int i = 0; i < num; i++ )
	{
		addFont( list, xFonts[i] );
	}

	XFreeFontNames( xFonts );
}

void KGeneral::addFont( QStrList &list, const char *xfont )
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

		list.append( font );
	}
}

void KGeneral::readSettings( int )
{

	QString str;
	
	generalFont = QFont( "helvetica", 12, QFont::Normal );

	KConfig config;
	config.setGroup( "General Font" );

	
	int num = config.readNumEntry( "Charset",-1 );
	if ( num>=0 )
		generalFont.setCharSet((QFont::CharSet)num);

	str = config.readEntry( "Family" );
	if ( !str.isNull() )
		generalFont.setFamily(str.data());
		
		
	str = config.readEntry( "Point Size" );
		if ( !str.isNull() )
		generalFont.setPointSize(atoi(str.data()));
		
	
	str = config.readEntry( "Weight" );
		if ( !str.isNull() )
	generalFont.setWeight(atoi(str.data()));
		
	
	str = config.readEntry( "Italic" );
		if ( !str.isNull() )
			if ( atoi(str.data()) != 0 )
				generalFont.setItalic(True);
	
		
	config.setGroup( "GUI Style" );

	str = config.readEntry( "Style" );
	if ( !str.isNull() )
	{
		if( str == "Windows 95" ) {
			applicationStyle=WindowsStyle;
		}
		else {
			applicationStyle=MotifStyle;
		}
	} else {
		applicationStyle=MotifStyle;
	}

}

void KGeneral::writeSettings()
{
	if ( !changed )
		return;
		
	KConfig *systemConfig = kapp->getConfig();
	
	systemConfig->setGroup( "General Font" );


	systemConfig->writeEntry("Family", generalFont.family(), true, true);
	
	QString pointSizeStr(10);
	pointSizeStr.sprintf("%d", generalFont.pointSize() );
	systemConfig->writeEntry("Point Size", pointSizeStr, true, true);

	QString charsetStr(10);
	charsetStr.sprintf("%d", generalFont.charSet() );
	systemConfig->writeEntry("Charset", charsetStr, true, true);
	
	QString weightStr(10);
	weightStr.sprintf("%d", generalFont.weight() );
	systemConfig->writeEntry("Weight", weightStr, true, true);
	
	QString italicStr(10);
	italicStr.sprintf("%d", (int)generalFont.italic() );
	systemConfig->writeEntry("Italic", italicStr, true, true);


	systemConfig->setGroup( "GUI Style" );

	QString str;
	if(applicationStyle==WindowsStyle)
		str.sprintf("Windows 95" );
	else
		str.sprintf("Motif" );
	systemConfig->writeEntry("Style", str, true, true);
	systemConfig->sync();
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


void KGeneral::slotPreviewFont( int indx )
{
	if ( indx == 0 )
		example_label->setFont( generalFont );
}

void KGeneral::slotHelp()
{
	kapp->invokeHTMLHelp( "kdisplay/kdisplay-6.html", "" );
}


void KGeneral::loadSettings()
{
  readSettings();

	if(applicationStyle==WindowsStyle)
		stCombo->setCurrentItem(1);
	else
		stCombo->setCurrentItem(0);

	fontTypeList->setCurrentItem( 0 );

	QStrListIterator it( fontList );
	for ( int i = 0; it.current(); ++it, i++ )
	{
		if ( !strcmp( generalFont.family(), it.current() ) )
			fontCombo->setCurrentItem( i );
	}
	example_label->setFont(generalFont);

	if ( generalFont.pointSize() <= 10 )
		((QRadioButton *)btnGrp->find( 0 ))->setChecked( TRUE );
	else if ( generalFont.pointSize() <= 12 )
		((QRadioButton *)btnGrp->find( 1 ))->setChecked( TRUE );
	else
		((QRadioButton *)btnGrp->find( 2 ))->setChecked( TRUE );

	cb1->setChecked( generalFont.bold() );
	
	cb2->setChecked( generalFont.italic() );

	QFont::CharSet charset=generalFont.charSet();

	for(int i = 0; i<CHARSETS_COUNT; i++){
	  if (charset==charsetsIds[i]){
	    charset_combo->setCurrentItem(i);
	    break;
	  }
	}

}

void KGeneral::applySettings()
{
  writeSettings();
  apply(TRUE);
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
