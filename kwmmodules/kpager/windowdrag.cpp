/****************************************
 *
 *  windowdrag.cpp  - The windowDrag object, used to drag windows across
 *     	                desktops
 *  Copyright (C) 1998  Antonio Larrosa Jimenez
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Send comments and bug fixes to antlarr@arrakis.es
 *  or to Antonio Larrosa, Rio Arnoya, 10 5B, 29006 Malaga, Spain
 *
 */
#include "windowdrag.h"
#include <stdio.h>

#if QT_VERSION >= 141

windowDrag::windowDrag(Window w,int deltax,int deltay, int origdesk,QWidget *parent)
    : QStoredDrag("application/x-kpager",parent,"windowdrag")
{
    char *tmp=new char[200];
    sprintf(tmp,"%ld %d %d %d",w,deltax,deltay,origdesk);
    QByteArray data(strlen(tmp)+1);
    data.assign(tmp,strlen(tmp)+1);

    setEncodedData(data);
}

windowDrag::~windowDrag()
{
}

bool windowDrag::canDecode (QDragMoveEvent *e)
{
    return e->provides("application/x-kpager");
}

bool windowDrag::decode( QDropEvent *e, Window &w,int &deltax,int &deltay,int &origdesk)
{
    QByteArray data=e->data("application/x-kpager");
    if (data.size())
    {
        char *tmp=data.data();
        sscanf(tmp,"%ld %d %d %d",&w,&deltax,&deltay,&origdesk);
        e->accept();
        return TRUE;
    }
    return FALSE;
}

#endif