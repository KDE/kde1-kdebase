/**********************************************************************

	--- Dlgedit generated file ---

	File: DesktopPathDialogData.cpp
	Last generated: Tue Jul 15 18:07:10 1997

	DO NOT EDIT!!!  This file will be automatically
	regenerated by dlgedit.  All changes will be lost.

 *********************************************************************/

#include "DesktopPathDialogData.h"
#include <kapp.h>

#define Inherited QDialog

#include <qlabel.h>

DesktopPathDialogData::DesktopPathDialogData
(
	QWidget* parent,
	const char* name
)
	:
	Inherited( parent, name, TRUE )
{
	QLabel* tmpQLabel;
	tmpQLabel = new QLabel( this, "Label_1" );
	tmpQLabel->setGeometry( 16, 16, 256, 24 );
	{
		QFont font( "helvetica", 12, 63, 0 );
		font.setStyleHint( (QFont::StyleHint)0 );
		font.setCharSet( (QFont::CharSet)0 );
		tmpQLabel->setFont( font );
	}
	tmpQLabel->setText( klocale->translate("Path for KDE Desktop Entries is not set.") );
	tmpQLabel->setAlignment( 289 );
	tmpQLabel->setMargin( -1 );

	tmpQLabel = new QLabel( this, "Label_2" );
	tmpQLabel->setGeometry( 16, 40, 192, 24 );
	{
		QFont font( "helvetica", 12, 63, 0 );
		font.setStyleHint( (QFont::StyleHint)0 );
		font.setCharSet( (QFont::CharSet)0 );
		tmpQLabel->setFont( font );
	}
	tmpQLabel->setText( klocale->translate("I suggest the following entries:") );
	tmpQLabel->setAlignment( 289 );
	tmpQLabel->setMargin( -1 );

	tmpQLabel = new QLabel( this, "Label_3" );
	tmpQLabel->setGeometry( 16, 72, 40, 24 );
	tmpQLabel->setText( klocale->translate("Path:") );
	tmpQLabel->setAlignment( 289 );
	tmpQLabel->setMargin( -1 );

	le_path = new QLineEdit( this, "LineEdit_1" );
	le_path->setGeometry( 24, 96, 360, 24 );
	le_path->setText( "" );
	le_path->setMaxLength( 32767 );
	le_path->setEchoMode( QLineEdit::Normal );
	le_path->setFrame( TRUE );

	tmpQLabel = new QLabel( this, "Label_4" );
	tmpQLabel->setGeometry( 16, 128, 88, 24 );
	tmpQLabel->setText( klocale->translate("PersonalPath:") );
	tmpQLabel->setAlignment( 289 );
	tmpQLabel->setMargin( -1 );

	le_ppath = new QLineEdit( this, "LineEdit_2" );
	le_ppath->setGeometry( 24, 152, 360, 24 );
	le_ppath->setText( "" );
	le_ppath->setMaxLength( 32767 );
	le_ppath->setEchoMode( QLineEdit::Normal );
	le_ppath->setFrame( TRUE );

	b_ok = new QPushButton( this, "PushButton_1" );
	b_ok->setGeometry( 152, 200, 100, 30 );
	b_ok->setText( klocale->translate("Ok") );
	b_ok->setAutoRepeat( FALSE );
	b_ok->setAutoResize( FALSE );
	b_ok->setAutoDefault( TRUE );

	resize( 400, 248 );
}


DesktopPathDialogData::~DesktopPathDialogData()
{
}
