/****************************************
 *
 *   desktop.h  - The Desktop widget
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

#include <qwidget.h>
#include <qdropsite.h>
#include <qlist.h>
#include <kwm.h>
#include <kwmmapp.h>
#include <kpixmap.h>
#include "windowdrag.h"

struct WindowProperties
{
    Window id;
    QString name;
    bool active;
    bool iconified;
    QRect geometry;
    QRect minigeometry;
    QRect framegeometry;
    QRect miniframegeometry;
    QPixmap *icon;
    QPixmap *pixmap;
    QPixmap *bigPixmap; // well, not so big :-)
};

class Desktop : public QWidget , QDropSite
{
    Q_OBJECT

private:
    int id;
    QFont *desktopfont;
    QPixmap *tmpScreen;
    
    QList <WindowProperties> *window_list;

    int screen_width, screen_height;
    int ratio;

    bool desktopActived;
    static int headerHeight;
    
    enum drawModes { plain = 0, icon = 1, pixmap = 2} drawWinMode;

    WindowProperties *getWindowProperties(Window w);
    int getIndexOfWindow(Window w);
    WindowProperties *windowAtPosition(const QPoint *p,bool *ok=0L,QPoint *pos=0L);

    void drawSunkRect(QPainter *p,int x,int y,int w,int h,bool highlighted);

    /** Background related stuff **/
    QPixmap *bigBackgroundPixmap;
    int backPixmapWidth; // These variabels contains the original size
    int backPixmapHeight; // of the background pixmap.
    
    QPixmap *backgroundPixmap;
    bool useBackgroundInfoFromKbgndwm;
    bool useWallpaper;
    QColor color1;
    QColor color2;
    uint pattern[8];
    
    enum { Tiled = 1, Centred, Scaled};
    int wpMode;
    
    enum { Flat = 1, Gradient, Pattern};
    int gfMode;
    
    enum { Portrait = 1,Landscape };
    int orMode;
    
    void readBackgroundSettings(void);     // read kbgndwm settings
    void prepareBackground(void);          // fills the backgroundPixmap
    QPixmap *loadWallpaper(QString wallpaper);

    void grabDesktop(void);
    void grabWindowContents(WindowProperties *wp);

protected:
    
    virtual void paintWindow(QPainter *painter,WindowProperties *wp, QRect &tmp);

public:
    Desktop(int id,int swidth,int sheight, QWidget *parent, char *name);
    virtual ~Desktop();

    bool contains(Window w);

    void setDesktopActived(bool status); // [De]Activate desktop

    void setRatio(int _ratio);
    void clearWindowList(void);
    void addWindow(Window w, int pos=-1);
    void removeWindow(Window w);
    void changeWindow(Window w);
    void raiseWindow(Window w);
    void lowerWindow(Window w);
    void activateWindow(Window w);



    void reconfigure(void); // Reads again the kbgndwm settings
    void setDrawMode(int mode); // sets the mode to draw windows

    static int getHeaderHeight(void);
    
    virtual void paintEvent(QPaintEvent *);

    virtual void mousePressEvent (QMouseEvent *e);
#if QT_VERSION >= 141
    virtual void dragEnterEvent(QDragEnterEvent *e);
    virtual void dragMoveEvent   (QDragMoveEvent *e);
    virtual void dropEvent       (QDropEvent *e);
#endif
    virtual void mouseDoubleClickEvent( QMouseEvent *);
    virtual void resizeEvent (QResizeEvent *);

    static KWMModuleApplication *kwmmapp;
signals:
    void moveWindow(Window w,int desktop,int x,int y, int origdesk);
    void switchToDesktop(int id);
};
    
