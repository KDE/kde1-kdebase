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

#ifndef __FINDDLG_H__
#define __FINDDLG_H__

#include <qdialog.h>
#include <qregexp.h>

class QLineEdit;

class KFindTextDialog : public QDialog
{
    Q_OBJECT

public:
    KFindTextDialog( QWidget *parent = 0, const char *name = 0 );

    const QRegExp &regExp() const { return rExp; }

    virtual void show();

signals:
    /*
     * connect to me to find out when the user has pressed the find button.
     */
    void find( const QRegExp &re );

public slots:
    void slotTextChanged( const char *t );
    void slotCase( bool c );
    void slotClose();
    void slotFind();

protected:
    QRegExp rExp;
    QLineEdit * edit;
};

#endif

