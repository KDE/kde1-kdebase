/* This file is part of the KDE libraries
    Copyright (C) 1998 Martin Jones (mjones@kde.org)

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

#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <kapp.h>
#include "finddlg.h"

#include "finddlg.moc"

KFindTextDialog::KFindTextDialog( QWidget *parent, const char *name )
    : QDialog( parent, name )
{
    // mimimise initial size of dialog
    resize( 0, 0 );

    QVBoxLayout *vl = new QVBoxLayout( this, 15 );

    QHBoxLayout *hl = new QHBoxLayout( 15 );
    vl->addLayout( hl );

    QLabel *label = new QLabel( i18n( "Find:" ), this );
    label->setFixedSize( label->sizeHint() );

    edit = new QLineEdit( this );
    edit->setFixedHeight( edit->sizeHint().height() );
    edit->setFocus();
    connect( edit, SIGNAL( textChanged( const char * ) ),
		    SLOT( slotTextChanged( const char * ) ) );

    hl->addWidget( label );
    hl->addWidget( edit );

    hl = new QHBoxLayout;
    vl->addLayout( hl );

    QCheckBox *cb = new QCheckBox( i18n( "Case &sensitive" ), this );
    connect( cb, SIGNAL( toggled( bool ) ), this, SLOT( slotCase( bool ) ) );
    cb->setFixedSize( cb->sizeHint() );
    hl->addWidget( cb );

    hl->addStretch();

    hl = new QHBoxLayout( 15 );
    vl->addLayout( hl );

    QPushButton *btn = new QPushButton( i18n( "&Find" ), this );
    btn->setFixedSize( btn->sizeHint() );
    btn->setDefault( true );
    connect( btn, SIGNAL( clicked() ), this, SLOT( slotFind() ) );
    hl->addWidget( btn );

    hl->addStretch();

    btn = new QPushButton( i18n( "&Close" ), this );
    btn->setFixedSize( btn->sizeHint() );
    connect( btn, SIGNAL( clicked() ), this, SLOT( slotClose() ) );
    hl->addWidget( btn );

    vl->activate();

    rExp.setCaseSensitive( false );
}

void KFindTextDialog::slotTextChanged( const char *t )
{
    rExp = t;
}

void KFindTextDialog::slotCase( bool c )
{
    rExp.setCaseSensitive( c );
}

void KFindTextDialog::slotClose()
{
    hide();
}

void KFindTextDialog::slotFind()
{
    emit find( rExp );
}

void KFindTextDialog::show()
{
    edit->selectAll();
    QDialog::show();
}

