/* -------------------------------------------------------------

   toplevel.h (part of Klipper - Cut & paste history for KDE)

   (C) by Andrew Stanley-Jones

   Generated with the KDE Application Generator

 ------------------------------------------------------------- */


#ifndef _TOPLEVEL_H_
#define _TOPLEVEL_H_

#include <kapp.h>
#include <ktmainwindow.h>
#include <qpopmenu.h>
#include <qclipboard.h>
#include <qintdict.h>
#include <qtimer.h>
#include <qpixmap.h>

class TopLevel : public KTMainWindow
{
  Q_OBJECT
  
public:
    TopLevel();
    ~TopLevel();
    

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);

private slots:
    void newClipData();
    void clickedMenu(int);

private:
    QString QSlast;
    QPopupMenu *pQPMmenu;
    QIntDict<QString> *pQIDclipData;
    QTimer *pQTcheck;
    QPixmap *pQPpic;
};

#endif

