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
#include <qbuttongroup.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <kapp.h>
#include <kcharsets.h>
#include <kconfigbase.h>
#include <ksimpleconfig.h>
#include <kwm.h>
#include <kcolordlg.h>

#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include "fontchooser.h"
#include "kresourceman.h"

#include "general.h"
#include "general.moc"

extern int dropError(Display *, XErrorEvent *);

FontUseItem::FontUseItem( const char *n, QFont default_fnt, bool f )
	: selected(0)
{
	_text = n;
	_default = _font = default_fnt;
	fixed = f;
}

QString FontUseItem::fontString( QFont rFont )
{
	QString aValue;
#if QT_VERSION > 140
	aValue = rFont.rawName();
	return aValue;
#endif
	QFontInfo fi( rFont );
	
	aValue.sprintf( "-*-" );
	aValue += fi.family();
	
	if ( fi.bold() )
		aValue += "-bold";
	else
		aValue += "-medium";
	
	if ( fi.italic() )
		aValue += "-i";
	else
		aValue += "-r";
		
	//aValue += "-normal-*-*-";
	//	
	//QString s;
	//s.sprintf( "%d-*-*-*-*-", fi.pointSize()*10 );
	//aValue += s;
	
	aValue += "-normal-*-";
		
	QString s;
	s.sprintf( "%d-*-*-*-*-*-", fi.pointSize() );
	aValue += s;
	
	
	switch ( fi.charSet() ) {
		case QFont::Latin1:
			aValue += "iso8859-1";
			break;
		case QFont::AnyCharSet:
		default:
			aValue += "-*";
			break;
		case QFont::Latin2:
			aValue += "iso8859-2";
			break;
		case QFont::Latin3:
			aValue += "iso8859-3";
			break;
		case QFont::Latin4:
			aValue += "iso8859-4";
			break;
		case QFont::Latin5:
			aValue += "iso8859-5";
			break;
		case QFont::Latin6:
			aValue += "iso8859-6";
			break;
		case QFont::Latin7:
			aValue += "iso8859-7";
			break;
		case QFont::Latin8:
			aValue += "iso8859-8";
			break;
		case QFont::Latin9:
			aValue += "iso8859-9";
			break;
	}
	return aValue;
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
	KConfigBase *config=NULL;

	if ( _rcfile.isEmpty() ) {
		config  = kapp->getConfig();
	} else {
		QString s( KApplication::localconfigdir() );
		s += "/";
		s += _rcfile;
		config = new KSimpleConfig( s.data(), true );
	}

	config->setGroup( _rcgroup.data() );
	
	QFont tmpFnt( _font );
	_font = config->readFontEntry( _rckey.data(), &tmpFnt );
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
	if ( _rcfile.isEmpty() ) {
		 config->writeEntry( _rckey.data(), _font, true, true );
	} else {
		config->writeEntry( _rckey.data(), _font );
	}
	
	 config->sync();
	
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

const char * KIconStyle::appName [] = {"kpanel", "kfm", "KDE"};
const int KIconStyle::nApp = 3;

KIconStyle::KIconStyle( QWidget *parent, QBoxLayout * topLayout )
{
    const char *appTitle [] = { i18n("Panel"),
                                i18n("File manager and desktop icons"),
                                i18n("Other") };

    m_dictCBNormal.setAutoDelete(true);
    m_dictCBLarge.setAutoDelete(true);
    m_dictSettings.setAutoDelete(true);

    QGroupBox * gb = new QGroupBox ( i18n("Icon style"), parent );
    topLayout->addWidget( gb );

    // The layout containing the checkboxes
    QGridLayout * gLayout = new QGridLayout( gb, 1+nApp, 4, 15 /*autoborder*/);
    gLayout->setColStretch(3, 20); // avoid cb moving when resizing

    // The two labels on row 0
    QLabel * label = new QLabel( i18n("Normal"), gb );
    label->setMinimumSize(label->sizeHint());
    gLayout->addWidget( label, 0, 1, AlignHCenter );

    label = new QLabel( i18n("Large"), gb );
    label->setMinimumSize(label->sizeHint());
    gLayout->addWidget( label, 0, 2, AlignHCenter );

    // The label + 2 checkboxes on each row
    QRadioButton * cb;
    for (int i = 0 ; i < nApp ; i++)
    {
        QButtonGroup * group = new QButtonGroup( );
        label = new QLabel( appTitle[i], gb );
        label->setMinimumSize(label->sizeHint());
        gLayout->addWidget( label, i+1, 0 );

        cb = new QRadioButton( gb, "" );
        group->insert(cb);
        cb->setMinimumSize(cb->sizeHint());
        gLayout->addWidget( cb, i+1, 1, AlignHCenter );
        m_dictCBNormal.insert( appName[i], cb ); // store the cb in the dict

        cb = new QRadioButton( gb, "" );
        group->insert(cb);
        cb->setMinimumSize(cb->sizeHint());
        gLayout->addWidget( cb, i+1, 2, AlignHCenter );
        m_dictCBLarge.insert( appName[i], cb ); // store the cb in the dict
    }
}

KIconStyle::~KIconStyle()
{
}

void KIconStyle::apply()
{
    bool changed = false;
    for (int i = 0 ; i < nApp ; i++)
    {
        QString s = m_dictCBNormal[ appName[i] ] -> isChecked() ? "Normal" : "Large";
        // See if the settings have changed
        if ( strcmp( (char*) m_dictSettings [ appName[i] ], s) != 0 )
        {
            // Store new setting
            char * setting = new char [s.length()];
            strcpy( setting, s );
            m_dictSettings.replace( appName[i], setting );
            // Apply it
            if ( strcmp( appName[i], "kpanel" ) == 0 )
              KWM::sendKWMCommand("kpanel:restart");
            else
              changed = true;
        }
    }
    if (changed)
      QMessageBox::information( 0L, i18n("Icons style"), i18n("The icon style change will not all be applied until you restart KDE."));
}

void KIconStyle::readSettings()
{
    KConfig config; // global config (.kderc)
    config.setGroup( "KDE" );
    for (int i = 0 ; i < nApp ; i++)
    {
        QString s = config.readEntry( QString(appName[i])+"IconStyle", "Normal" );
        m_dictCBNormal[ appName[i] ] -> setChecked( s == "Normal");
        m_dictCBLarge[ appName[i] ] -> setChecked( s == "Large" );
        char * setting = new char [s.length()];
        strcpy( setting, s );
        m_dictSettings.insert( appName[i], setting ); // store initial value
    }    
}

void KIconStyle::writeSettings()
{
    KConfig * cfg = kapp->getConfig();
    cfg->setGroup( "KDE" );
    for (int i = 0 ; i < nApp ; i++)
    {
        QString s = m_dictCBNormal[ appName[i] ] -> isChecked() ? "Normal" : "Large";
        cfg->writeEntry( QString(appName[i])+"IconStyle", s, 
                            true, true /* global setting (.kderc) */ );
        if (!strcmp(appName[i], "kpanel"))
        {
          KConfig * config = new KConfig(KApplication::kde_configdir() + "/kpanelrc",
                                         KApplication::localconfigdir() + "/kpanelrc");
          config->setGroup("kpanel"); 
          // Special case for kpanel, as asked by Torsten :
          // Sync kpanel's size with icon size
          // Awful duplicated code from kcontrol/panel/panel.cpp
          // I will get killed by others developers...
          if (s == "Normal")
          {
            config->writeEntry("Style", "normal");
            config->writeEntry("BoxWidth",45);
            config->writeEntry("BoxHeight",45);
            config->writeEntry("Margin",0);
            config->writeEntry("TaskbarButtonHorizontalSize",4);
            //config->writeEntry("DesktopButtonFont","*-helvetica-medium-r-normal--12-*");
            config->writeEntry("DesktopButtonRows",2);
            //config->writeEntry("DateFont","*-times-medium-i-normal--12-*"); 
          } else {
            config->writeEntry("Style", "large");
            config->writeEntry("BoxWidth",52);
            config->writeEntry("BoxHeight",52);
            config->writeEntry("Margin",2);
            config->writeEntry("TaskbarButtonHorizontalSize",4);
            //config->writeEntry("DesktopButtonFont","*-helvetica-medium-r-normal--14-*");
            config->writeEntry("DesktopButtonRows",2);
            //config->writeEntry("DateFont","*-times-bold-i-normal--12-*"); 
          }
          config->sync();
          delete config;
        }
    }
    cfg->sync();
}

void KIconStyle::setDefaults()
{
    for (int i = 0 ; i < nApp ; i++)
    {
        m_dictCBNormal[ appName[i] ] -> setChecked( true );
        m_dictCBLarge[ appName[i] ] -> setChecked( false );
    }
}

//------------------------------------------------------------------

KGeneral::KGeneral( QWidget *parent, int mode, int desktop )
	: KDisplayModule( parent, mode, desktop )
{
	changed = false;
	useRM = true;
	macStyle = false;//CT
	iconStyle = 0L;

	//debug("KGeneral::KGeneral");
	
	// if we are just initialising we don't need to create setup widget
	if ( mode == Init )
		return;
	
	kde_display = x11Display();
	KDEChangeGeneral = XInternAtom( kde_display, "KDEChangeGeneral", False);
	screen = DefaultScreen(kde_display);
	root = RootWindow(kde_display, screen);

	setName( i18n("Style") );

	readSettings();
	
	QBoxLayout *topLayout = new QVBoxLayout( this, 10 );
	//CT	QBoxLayout *top2Layout = new QHBoxLayout();
	//CT	topLayout->addLayout(top2Layout);
	
	cbStyle = new QCheckBox( i18n( "&Draw widgets in the style of Windows 95" ),
				 this );
	cbStyle->adjustSize();
	cbStyle->setMinimumSize(cbStyle->size());

	if( applicationStyle == WindowsStyle )
	        cbStyle->setChecked( true );
	else
		cbStyle->setChecked( false);
	
	connect( cbStyle, SIGNAL( clicked() ), SLOT( slotChangeStyle()  )  );
	
	topLayout->addWidget( cbStyle, 10 );//CT


	//CT 30Nov1998
	cbMac = new QCheckBox( i18n( "&Menubar on top of the screen in the style of MacOS" ),
				 this );
	cbMac->adjustSize();
	cbMac->setMinimumSize(cbMac->size());

	if( macStyle )
	    cbMac->setChecked( true );
	else
	    cbMac->setChecked( false);
	
	connect( cbMac, SIGNAL( clicked() ), SLOT( slotMacStyle()  )  );
	
	topLayout->addWidget( cbMac, 10 );

	cbRes = new QCheckBox( i18n( "&Apply fonts and colors to non-KDE apps" ),
				 this );
	cbRes->setMinimumSize(cbRes->sizeHint());

	if( useRM )
	        cbRes->setChecked( true );
	else
		cbRes->setChecked( false);
	
	connect( cbRes, SIGNAL( clicked() ), SLOT( slotUseResourceManager()  )  );
	
	topLayout->addWidget( cbRes, 10 );//CT

	iconStyle = new KIconStyle( this, topLayout ); // DF

	topLayout->addStretch( 100 );
	topLayout->activate();

	iconStyle->readSettings(); // DF
}


void KGeneral::slotChangeStyle()
{
	if( cbStyle->isChecked() )
		applicationStyle = WindowsStyle;
	else
		applicationStyle = MotifStyle;
				
	changed=true;
}

void KGeneral::slotUseResourceManager()
{
	useRM = cbRes->isChecked();
		
	changed=true;
}

//CT 30Nov1998 - mac style set
void KGeneral::slotMacStyle()
{
	macStyle = cbMac->isChecked();
		
	changed=true;
}
//CT

KGeneral::~KGeneral()
{
	delete iconStyle; // DF
}



void KGeneral::readSettings( int )
{		
	QString str;

	KConfig config;
	config.setGroup( "KDE" );

	str = config.readEntry( "widgetStyle", "Windows 95" );
	if ( str == "Windows 95" )
		applicationStyle = WindowsStyle;
	else
		applicationStyle = MotifStyle;

	//CT 30Nov1998 - mac style set

	str = config.readEntry( "macStyle", "off");
	if ( str == "on" )
	  macStyle = true;
	else
	  macStyle = false;
	//CT

		
	KConfigGroupSaver saver(kapp->getConfig(), "X11");
	useRM = kapp->getConfig()->readBoolEntry( "useResourceManager", false );
}

void KGeneral::setDefaults()
{
	cbStyle->setChecked( true );
	cbRes->setChecked( true );
	cbMac->setChecked( false );//CT
	useRM = true;
	macStyle = false;//CT
	slotChangeStyle();
	slotMacStyle();//CT
	iconStyle->setDefaults(); // DF
}

void KGeneral::defaultSettings()
{
	setDefaults();
}

void KGeneral::writeSettings()
{
	iconStyle->writeSettings(); // DF
	if ( !changed )
		return;
		
	KConfig *config = kapp->getConfig();
	config->setGroup( "KDE" );

	QString str;
	if( applicationStyle == WindowsStyle )
		str.sprintf("Windows 95" );
	else
		str.sprintf("Motif" );
	config->writeEntry("widgetStyle", str, true, true);

	//CT 30Nov1998 - mac style set
	config->writeEntry( "macStyle", macStyle?"on":"off", true, true);
	//CT

	KConfigGroupSaver saver(kapp->getConfig(), "X11");
	kapp->getConfig()->writeEntry( "useResourceManager", useRM );

	config->sync();
	
	if ( useRM ) {
		QApplication::setOverrideCursor( waitCursor );

		// the krdb run is for colors and other parameters (Matthias)
		KProcess proc;
		proc.setExecutable("krdb");
		proc.start( KProcess::Block );

		// still some KResourceMan stuff stuff. We need to
		// clean this up.  The best thing would be to use
		// KResourceMan always for KDE applications to make
		// the desktop settings machine independent but
		// per-display (Matthias)
		KResourceMan *krdb = new KResourceMan();
		krdb->setGroup( "KDE" );
		krdb->writeEntry( "widgetStyle", str );
		krdb->sync();

		QApplication::restoreOverrideCursor();
	}
	

	if (macStyle) {
	    KWM::sendKWMCommand("macStyleOn");
	}
	else {
	    KWM::sendKWMCommand("macStyleOff");
	}
	QApplication::syncX();
	

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
    XFree((char*) *p);
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
	iconStyle->apply(); // DF
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
	
	XFree((char *) rootwins);
	
	changed=false;
}


void KGeneral::slotHelp()
{
	kapp->invokeHTMLHelp( "kcmdisplay/kdisplay-7.html", "" );
}

void KGeneral::applySettings()
{
  writeSettings();
  apply( true );
}





//------------------------------------------------------------------

KFonts::KFonts( QWidget *parent, int mode, int desktop )
	: KDisplayModule( parent, mode, desktop )
{
	int i;
	changed = false;

	//debug("KFonts::KFonts");
	
	// if we are just initialising we don't need to create setup widget
	if ( mode == Init )
		return;
	
	kde_display = x11Display();
	KDEChangeGeneral = XInternAtom( kde_display, "KDEChangeGeneral", False);
	screen = DefaultScreen(kde_display);
	root = RootWindow(kde_display, screen);

	setName( i18n("Fonts") );

	readSettings();
	
	QBoxLayout *topLayout = new QVBoxLayout( this, 10 );
	
	topLayout->addSpacing( 15 );
	
	QBoxLayout *pushLayout = new QHBoxLayout( 5 );
	
	topLayout->addLayout( pushLayout );
	
	FontUseItem *item = new FontUseItem( i18n("General font"),
				QFont( "helvetica", 12 ) );
	item->setRC( "General", "font" );
	fontUseList.append( item );
	
	item = new FontUseItem( i18n("Fixed font"),
				QFont( "fixed", 10 ), true );
	item->setRC( "General", "fixedFont" );
	fontUseList.append( item );
	
	//CT 03Nov1998 - this code was here already. Only reactivated
	item = new FontUseItem( i18n("Window title font"),
				QFont( "helvetica", 12, QFont::Bold ) );
	item->setRC( "WM", "titleFont" );
	fontUseList.append( item );
				
	item = new FontUseItem( i18n("Panel button font"),
				QFont( "helvetica", 12 )  );
	item->setRC( "kpanel", "DesktopButtonFont", "kpanelrc" );
	fontUseList.append( item );
	
	item = new FontUseItem( i18n("Panel clock font"),
				QFont( "helvetica", 12, QFont::Normal) );
	item->setRC( "kpanel", "DateFont", "kpanelrc" );
	fontUseList.append( item );
	
	for ( i = 0; i < (int) fontUseList.count(); i++ )
		fontUseList.at( i )->readFont();
	
	lbFonts = new QListBox( this );
	for ( i=0; i < (int) fontUseList.count(); i++ )
  	     lbFonts->insertItem( fontUseList.at( i )->text() );
	lbFonts->adjustSize();
	lbFonts->setMinimumSize(lbFonts->size());
	
	connect( lbFonts, SIGNAL( highlighted( int ) ),
		 SLOT( slotPreviewFont( int ) ) );
			
	pushLayout->addWidget( lbFonts );
	
	fntChooser = new KFontChooser( this );
	
	connect( fntChooser, SIGNAL( fontChanged( QFont ) ), this,
		SLOT( slotSetFont( QFont ) ) );
	
	pushLayout->addWidget( fntChooser );
	
	QBoxLayout *stackLayout = new QVBoxLayout( 4 );
	
	topLayout->addLayout( stackLayout );

	QLabel *label = new QLabel( i18n("Sample text"), this );
	label->adjustSize();
	label->setFixedHeight( label->height() );
	label->setMinimumWidth(label->width());

	stackLayout->addWidget( label );
	
	lSample = new FontPreview( this );
	lSample->setText(i18n( "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG\n"
			  "the quick brown fox jumps over the lazy dog\n"
			  "0 1 2 3 4 5 6 7 8 9   ! \" £ $ % ^ & * ( )" ));
	lSample->setAlignment( AlignLeft | AlignVCenter );
	lSample->setFixedHeight( 2*lSample->sizeHint().height() );
	lSample->setFrameStyle( QFrame::Panel | QFrame::Sunken );
	lSample->adjustSize();
	lSample->setMinimumSize(lSample->size());

	stackLayout->addWidget( lSample );
	lbFonts->setCurrentItem( 0 );

	topLayout->activate();
}



KFonts::~KFonts()
{
}



void KFonts::readSettings( int )
{		
    useRM = kapp->getConfig()->readBoolEntry( "useResourceManager", false );
}

void KFonts::setDefaults()
{
	int ci = lbFonts->currentItem();
	for ( int i = 0; i < (int) fontUseList.count(); i++ )
		fontUseList.at( i )->setDefault();
	fontUseList.at( ci );
	slotPreviewFont( ci );
	
}

void KFonts::defaultSettings()
{
	setDefaults();
}

void KFonts::writeSettings()
{
	if ( !changed )
		return;
		
	for ( int i = 0; i < (int) fontUseList.count(); i++ )
		fontUseList.at( i )->writeFont();	
	

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
		krdb->sync();

		QApplication::restoreOverrideCursor();
	}
	
	fontUseList.at( lbFonts->currentItem() );
	
}

void KFonts::slotApply()
{
	writeSettings();
	apply();
}


void KFonts::apply( bool  )
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
	
	XFree((char *) rootwins);
	
	changed=false;
}

void KFonts::slotSetFont( QFont fnt )
{
	fontUseList.current()->setFont( fnt );
	lSample->setPreviewFont( fnt );
	changed = true;
}

void KFonts::slotPreviewFont( int index )
{
	fntChooser->setFont( fontUseList.at( index )->font(),
			fontUseList.at( index )->spacing() );
	lSample->setPreviewFont( fontUseList.at( index )->font() );
}

void KFonts::slotHelp()
{
	kapp->invokeHTMLHelp( "kcmdisplay/kdisplay-6.html", "" );
}

void KFonts::applySettings()
{
  writeSettings();
  apply( true );
}





