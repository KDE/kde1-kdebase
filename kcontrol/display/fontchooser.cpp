/* This file is part of the KDE libraries
    Copyright (C) 1998	Mark Donohoe <donohoe@kde.org>
						Stephan Kulow				  

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
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
		
	QLabel *label = new QLabel( cmbFont, i18n("&Typeface"), this );
	label->adjustSize();
	label->setMinimumSize( label->size() );
	
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
	sbSize->adjustSize();

	connect( sbSize, SIGNAL( valueDecreased() ),
		 SLOT( slotFontSize() ) );
		 
	connect( sbSize, SIGNAL( valueIncreased() ),
		 SLOT( slotFontSize() ) );
	
	label = new QLabel( sbSize, i18n("&Size"), this );
	label->setMinimumSize( label->sizeHint() );

	cmbCharset = new QComboBox( false, this );
	
	cmbCharset->adjustSize();
	cmbCharset->setInsertionPolicy( QComboBox::NoInsertion );
	connect( cmbCharset, SIGNAL( activated( const char * ) ),
		 SLOT( slotCharset( const char * ) ) );
	
	sbSize->setFixedHeight( cmbCharset->height() );
	sbSize->setMinimumWidth(sbSize->width());
	cmbCharset->setFixedHeight( cmbCharset->height() );
	cmbCharset->setMinimumWidth( cmbCharset->width());

	stackLayout->addWidget( label );
	stackLayout->addWidget( sbSize );
	
	stackLayout = new QVBoxLayout( 4 );
	
	pushLayout->addLayout( stackLayout, 30 );
	
	label = new QLabel( cmbCharset, i18n("&Character set"), this );
	label->adjustSize();
	label->setMinimumSize( label->size() );
	
	stackLayout->addWidget( label );
	stackLayout->addWidget( cmbCharset );

	topLayout->activate();
	fillCharsetCombo();
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
	fillCharsetCombo();
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
	// Use KDE fonts if there is a KDE font list and check that the fonts
	// exist on the server where the desktop is running.
	
	QStrList lstSys, lstKDE;
	
	if ( fixed ) {
		getFontList( lstSys, "-*-*-*-*-*-*-*-*-*-*-m-*-*-*" );
		getFontList( lstSys, "-*-*-*-*-*-*-*-*-*-*-c-*-*-*" );
	} else
		getFontList( lstSys, "-*-*-*-*-*-*-*-*-*-*-p-*-*-*" );
		
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

  emit fontChanged( fnt );
  changed=TRUE;
}

void KFontChooser::slotSelectFont( const char *fname )
{
//	if( lbFonts->currentItem() == 0 )
	fnt.setFamily( fname );
		
	fillCharsetCombo();	
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
