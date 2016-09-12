/*
    This file is part of the KDE File Manager

    Copyright (C) 1998 Waldo Bastian (bastian@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License 
    as published by the Free Software Foundation; either version 2 
    of the License, or (at your option) any later version.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this library; see the file COPYING. If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
//----------------------------------------------------------------------------
//
// KDE File Manager -- HTTP Cookie Dialogs
// $Id: kcookiewin.cpp,v 1.2 1998/12/21 17:36:36 bieker Exp $

#include "kcookiewin.h"
#include "kcookiejar.h"

#include "kcookiewin.moc"

#include <qdialog.h>
#include <qgrpbox.h>
#include <qbttngrp.h>
#include <qradiobt.h>
#include <qlabel.h>
#include <qframe.h>
#include <qpixmap.h>

#include <klocale.h>
#include <kapp.h>
#include <kbuttonbox.h>
        
KCookieWin::KCookieWin(QWidget *parent, KCookie *_cookie) :
    QDialog ( parent, "" , true )
{
    static bool icon_initialized = false;
    static QPixmap icon;
        
    if( !icon_initialized )
    {
        QString ipath = kapp->kde_datadir() + "/kfm/pics/";
        icon.load(ipath + "exclamation.xpm");
        icon_initialized = true;
    }

    cookie = _cookie;

    setCaption(i18n("Cookie Alert!"));

    int border = 8;

    layout = new QVBoxLayout(this, border);

    QHBoxLayout *layoutH = new QHBoxLayout( border );
    layout->addLayout( layoutH );

    QLabel *picture = new QLabel(this, "_pict" );
    picture->setPixmap(icon);
    picture->adjustSize();
    picture->setFixedSize( picture->size().width()+10, picture->size().height() );
    layoutH->addWidget( picture);

    QVBoxLayout *layoutV = new QVBoxLayout( border );
    layoutH->addLayout( layoutV );

    QLabel *msg = new QLabel( i18n("You received a cookie from host:"), this, "_msg" );
    msg->adjustSize();
    msg->setFixedSize( msg->size().width()+10, msg->size().height() );
    layoutV->addWidget(msg);

    QString str;
    str = cookie->getHost();
    msg = new QLabel( str.data(), this, "_msg" );
    msg->setAlignment( AlignHCenter );
    msg->adjustSize();
    msg->setFixedSize( msg->size().width()+10, msg->size().height() );
    layoutV->addWidget(msg);

    msg = new QLabel( i18n("Do you want to accept or reject this cookie?"), this, "_msg" );
    msg->adjustSize();
    msg->setFixedSize( msg->size().width()+10, msg->size().height() );
    layout->addWidget(msg);

    QButtonGroup *optGroup = new QButtonGroup( this, "options" );
    optGroup->hide();
    optGroup->setExclusive( true );

    QGridLayout *grid = new QGridLayout( 5, 3, 5 );
        
    layout->addLayout(grid);    
        
    grid->setRowStretch(0,5);
    grid->setRowStretch(1,10);
    grid->setRowStretch(2,10);
    grid->setRowStretch(3,10);
    grid->setRowStretch(4,5);
                                                        
    grid->setColStretch(0,0);
    grid->setColStretch(1,20);
    grid->setColStretch(2,10);
                                                                                        
    grid->addRowSpacing(0,5);
    grid->addRowSpacing(4,5);

    QFrame *f1 = new QFrame(this);
    f1->setLineWidth(1);
    f1->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    grid->addMultiCellWidget( f1, 0, 0, 0, 2 );

    msg = new QLabel( i18n("Apply to "), this, "_msg" );
    msg->adjustSize();
    msg->setFixedSize( msg->size().width(), msg->size().height() );
    grid->addWidget( msg, 2, 0 );
                            
    rb1 = new QRadioButton( i18n("&This cookie only"), this );
    rb1->adjustSize();
    rb1->setFixedHeight( rb1->height() );
    rb1->setMinimumWidth( rb1->width() );
    rb1->setChecked( true );
    grid->addWidget( rb1, 1, 1 );
    optGroup->insert( rb1 );
     
    rb2 = new QRadioButton( i18n("All cookies from this &domain"), this );
    rb2->adjustSize();
    rb2->setFixedHeight( rb2->height() );
    rb2->setMinimumWidth( rb2->width() );
    grid->addWidget( rb2, 2, 1 );
    optGroup->insert( rb2 );

    rb3 = new QRadioButton( i18n("All &cookies"), this );
    rb3->adjustSize();
    rb3->setFixedHeight( rb3->height() );
    rb3->setMinimumWidth( rb3->width() );
    grid->addWidget( rb3, 3, 1 );
    optGroup->insert( rb3 );

    f1 = new QFrame(this);
    f1->setLineWidth(1);
    f1->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    grid->addMultiCellWidget( f1, 4, 4, 0, 2 );

//    optGroup->adjustSize();
//    optGroup->setFixedHeight( optGroup->height() );
//    optGroup->setMinimumWidth( optGroup->width() );

    layout->addStretch( 10 );

    KButtonBox *bbox = new KButtonBox( this );
    bbox->addStretch( 20 );
                
    QPushButton *b0 = bbox->addButton( i18n("&Accept") );
    connect( b0, SIGNAL( clicked() ), this, SLOT( b0Pressed() ) );

    bbox->addStretch( 10 );

    QPushButton *b1 = bbox->addButton( i18n("&Reject") );
    connect( b1, SIGNAL( clicked() ), this, SLOT( b1Pressed() ) );

    bbox->addStretch( 20 );

    b1->setDefault( true );
    bbox->layout();
    layout->addWidget( bbox );
    layout->freeze();
}

KCookieWin::~KCookieWin()
{
}

void KCookieWin::b0Pressed()
{
    if (cookiejar)
    {
        if (rb2->isChecked())
            cookiejar->setDomainAdvice(cookie, KCookieAccept);
        else if (rb3->isChecked())
            cookiejar->setGlobalAdvice(KCookieAccept);
    }
    done( KCookieAccept );
}

void KCookieWin::b1Pressed()
{
    if (cookiejar)
    {
        if (rb2->isChecked())
            cookiejar->setDomainAdvice(cookie, KCookieReject);
        else if (rb3->isChecked())
            cookiejar->setGlobalAdvice(KCookieReject);
    }
    done( KCookieReject );
}



