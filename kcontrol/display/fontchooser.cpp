//
// A widget for choosing fonts
//
// Copyright (c)  Mark Donohoe 1998
//

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

#include "fontchooser.h"
#include "fontchooser.moc"

KFontChooser::KFontChooser( QWidget *parent, const char *name )
	: QWidget( parent, name )
{
	int i;
	
	fnt = QFont( "helvetica", 12 );
	changed = False;
	
	QBoxLayout *topLayout = new QVBoxLayout( this, 10, 5 );
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
	sbSize->setRange( 8, 16 );
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
		list = lstSys;
		return;
	}
	
	for( int i = 0; i < (int) lstKDE.count(); i++ ) {
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
