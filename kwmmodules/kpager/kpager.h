/****************************************
 *
 *   kpager.h  - The KPager main widget
 *   Copyright (C) 1998  Antonio Larrosa Jimenez
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Send comments and bug fixes to antlarr@arrakis.es
 *   or to Antonio Larrosa, Rio Arnoya, 10 5B, 29006 Malaga, Spain
 *
 */
#ifndef _KPAGER_H
#define _KPAGER_H

#include <ktmainwindow.h>
#include <kmenubar.h>
#include <kapp.h>
#include "kpagerclient.h"

class KApplication;
class KAccel;
class QPopupMenu;
class KPagerClient;

class KPager : public KTMainWindow
{
    Q_OBJECT
private:
    QPopupMenu *m_file;
    QPopupMenu *m_options;
    QPopupMenu *m_drawmode;
    QPopupMenu *m_help;
    KAccel *kKeysAccel;

    KPagerClient *kpagerclient;

    bool menubar_visible;


    Window selectedWindow;  // This is the window to operate on when calling
				// windowOperations

    virtual void saveProperties(KConfig *kcfg);
    virtual void readProperties(KConfig *kcfg);

    QPopupMenu *getOptionlikeMenu(void);
    QPopupMenu *getToDesktoplikeMenu(int mark);

protected:
    virtual void closeEvent ( QCloseEvent * );

public:

    KPager(KWMModuleApplication *kwmmapp,const char *name=0);
    virtual ~KPager();

public slots:

    void file_quit();
    void options_toggleMenuBar();
    void options_toggleGlobalDesktop();
    void options_2Rows();
    void options_oneClickMode();
    void options_showDesktopName();
    void options_drawPlain();
    void options_drawIcon();
    void options_drawPixmap();

    void showPopupMenu(Window w,QPoint p);

    void windowOperations(int id); 
    void toDesktop(int id);
    
private:
    KMenuBar *menu;
};

#endif
