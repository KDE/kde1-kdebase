
//
// KDE Display background setup module
//
// Copyright (c)  Martin R. Jones 1996
//
// Converted to a kcc module by Matthias Hoelzer 1997
//

#ifdef HAVE_CONFIG
#include <config.h>
#endif

#include <stdlib.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <qimage.h>
#include <qgrpbox.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qfiledlg.h>
#include <qradiobt.h>
#include <qchkbox.h>
#include <qpainter.h>
#include <qlayout.h>

#include <kapp.h>
#include <kwm.h>
#include <kiconloader.h>
#include <ksimpleconfig.h>
#include <kbuttonbox.h>

#include <X11/Xlib.h>

#include "kaccel.h"


#include "standard.h"
#include "standard.moc"


//----------------------------------------------------------------------------

KStdConfig::KStdConfig( QWidget *parent, const char *name )
	: KConfigWidget( parent, name )
{
	dict.setAutoDelete( false );
	keys = new KAccel( this );
	
	keys->insertItem(i18n("Quit"),"Quit" , "CTRL+Q");
	keys->insertItem(i18n("Open"),"Open" , "CTRL+O");
	keys->insertItem(i18n("New"), "New", "CTRL+N");
	keys->insertItem(i18n("Close"), "Close", "CTRL+W");
	keys->insertItem(i18n("Print"),"Print" , "CTRL+P");
	keys->insertItem(i18n("Help"), "Help", "F1");
	keys->insertItem(i18n("Next"), "Next", "Next");
	keys->insertItem(i18n("Prior"),"Prior" , "Prior");
	keys->insertItem(i18n("Cut"), "Cut", "CTRL+X");
	keys->insertItem(i18n("Copy"), "Copy", "CTRL+C");
	keys->insertItem(i18n("Paste"),"Paste" , "CTRL+V");
	keys->insertItem(i18n("Undo"),"Undo" , "CTRL+Z");
	keys->insertItem(i18n("Find"),"Find" , "CTRL+F");
	keys->insertItem(i18n("Replace"),"Replace" , "CTRL+R");
	keys->insertItem(i18n("Insert"), "Insert", "CTRL+Insert");
	keys->insertItem(i18n("Home"), "Home", "CTRL+Home");
	keys->insertItem(i18n("End"),"End" , "CTRL+End");
	
	debug("inserted keys");
	
	keys->setConfigGlobal( true );
	keys->readSettings();
	
	debug("read settings");
	
	QBoxLayout *topLayout = new QVBoxLayout( this, 10 );
	
	QBoxLayout *stackLayout = new QVBoxLayout( this, 4 );
	
	topLayout->addLayout( stackLayout );
	
	QListBox *lbSchemes = new QListBox( this );
	lbSchemes->insertItem( "Current scheme" );
	lbSchemes->insertItem( "KDE default" );
	lbSchemes->insertItem( "KDE lite" );
	
	lbSchemes->adjustSize();
	lbSchemes->setMinimumHeight( 2*lbSchemes->height() );
	
	QLabel *label = new QLabel( lbSchemes, "&Key scheme", this );
	label->adjustSize();
	label->setFixedHeight( label->height() );
	
	stackLayout->addWidget( label );
	stackLayout->addWidget( lbSchemes );
	
	dict = keys->keyDict();
	
	debug("got key dict");
	
	kc =  new KKeyChooser( &dict, this );
	
	debug("Make key chooser standard");
	
	topLayout->addWidget( kc, 10 );
	
	topLayout->activate();
	
	//keys->setKeyDict( dave );
}

void KStdConfig::resizeEvent( QResizeEvent * )
{
 
}

void KStdConfig::readSettings( )
{
  
}

void KStdConfig::writeSettings(  )
{
	debug("Writing key settings");
	keys->writeSettings();
}

void KStdConfig::getDeskNameList()
{
   
}

void KStdConfig::setDesktop( int desk )
{
   
}

void KStdConfig::showSettings()
{ 
   
}

void KStdConfig::slotApply()
{
	writeSettings();
}

void KStdConfig::apply( bool force )
{
	
}

void KStdConfig::retainResources() {
	
}

void KStdConfig::setMonitor()
{
   
    
}

// Attempts to load the specified wallpaper and creates a centred/scaled
// version if necessary.
// Note that centred pixmaps are placed on a full screen image of background
// color1, so if you want to save memory use a small tiled pixmap.
//
int KStdConfig::loadWallpaper( const char *name, bool useContext )
{
	
}

void KStdConfig::slotSelectColor1( const QColor &col )
{
	
}

void KStdConfig::slotSelectColor2( const QColor &col )
{

}

void KStdConfig::slotBrowse()
{
	
}

void KStdConfig::slotWallpaper( const char *filename )
{
   
}

void KStdConfig::slotWallpaperMode( int m )
{

}

void KStdConfig::slotColorMode( int m )
{
	
}

void KStdConfig::slotSetup2Color()
{
   
}

void KStdConfig::slotStyleMode( int m )
{
   
}

void KStdConfig::slotSwitchDesk( int num )
{
   
}

void KStdConfig::slotRenameDesk()
{
   
}

void KStdConfig::slotHelp()
{

}

void KStdConfig::loadSettings()
{
   
}

void KStdConfig::applySettings()
{
	debug("apply settings");
	debug("No. of items in dict %d", dict.count() );
	keys->setKeyDict( dict );
	debug("set key dict");
    writeSettings();
}
