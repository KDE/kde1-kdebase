/* -------------------------------------------------------------

   toplevel.cpp (part of Klipper - Cut & paste history for KDE)

   (C)  by

   Generated with the KDE Application Generator

 ------------------------------------------------------------- */


#include "toplevel.h"
#include <mykapp.h>
#include <kwm.h>
#include <qintdict.h>
#include <qpainter.h>
#include "qmessagebox.h"
#include "kiconloader.h"
#include <kkeydialog.h>

#define QUIT_ITEM   50
#define CONFIG_ITEM 60

/* XPM */
/* Drawn  by Andreas Thienemann for the K Desktop Environment */
/* See http://www.kde.org */
static const char*mouse[]={
"24 24 9 1",
" 	c None",
".	c #000000",
"+	c #C7C3C7",
"@	c #A6A2A6",
"#	c #FF8200",
"$	c #C75900",
"%	c #FFAA59",
"&	c #FFFFFF",
"*	c #DFDFDF",
"       ..........       ",
"    ....+++++++@....    ",
"   .####.@@@@@@.###$.   ",
"   .#%............%$..  ",
"   .#%.&&&&&&&&&*.%$..  ",
"   .#%.&........*.%$..  ",
"   .#%.&&&&&&&&&*.%$..  ",
"   .#%.&..&&&&&&*.%$..  ",
"   .#%.&&&&&&&&&*.%$..  ",
"   .#%.&.....&&&*.%$..  ",
"   .#%.&&&&&&&&&*.%$..  ",
"   .#%.&.&&&&&&&*.%$..  ",
"   .#%.&&&&&&&&&*.%$..  ",
"   .#%.&.&&.&&&&*.%$..  ",
"   .#%.&.&.&&&&&*.%$..  ",
"   .#%.&..&&&&&&*.%$..  ",
"   .#%.&.&.&&&&&*.%$..  ",
"   .#%.&.&&.&&&&*.%$..  ",
"   .#%.*****.****.%$..  ",
"   .#%............%$..  ",
"   .#%%%%%%%%%%%%%%$..  ",
"   .$$$$$$$$$$$$$$$$..  ",
"   ...................  ",
"    ..................  "};

TopLevel::TopLevel() /*FOLD00*/
  : KTMainWindow()
{
    QSlast = "";
    pQPMmenu = new QPopupMenu(0x0, "main_menu");
    connect(pQPMmenu, SIGNAL(activated(int)),
            this, SLOT(clickedMenu(int)));
    pQPMmenu->insertItem(i18n("Quit Clipboard History"), QUIT_ITEM);
    pQPMmenu->insertSeparator();
    pQPMmenu->insertItem(i18n("Configure Shortcut"), CONFIG_ITEM);
    pQPMmenu->insertSeparator();
    pQIDclipData = new QIntDict<QString>();
    pQIDclipData->setAutoDelete(TRUE);
    newClipData();
    pQTcheck = new QTimer(this, "timer");
    pQTcheck->start(1000, FALSE);
    connect(pQTcheck, SIGNAL(timeout()),
            this, SLOT(newClipData()));
    pQPpic = new QPixmap(mouse);

    globalKeys = new KGlobalAccel();
    globalKeys->insertItem(i18n("Select clipboard contents"),
				"select-clipboard", "CTRL+ALT+V");
    globalKeys->connectItem("select-clipboard", this, SLOT(globalKeyEvent()));
    globalKeys->readSettings();
}

TopLevel::~TopLevel()
{
    delete pQTcheck;
    delete pQPMmenu;
    delete pQIDclipData;
    delete pQPpic;
}

void TopLevel::mousePressEvent(QMouseEvent *) /*FOLD00*/
{
  pQPMmenu->move(-1000,-1000);
  pQPMmenu->show();
  pQPMmenu->hide();
  QRect g = KWM::geometry( this->winId() );
  if ( g.x() > QApplication::desktop()->width()/2 &&
       g.y()+pQPMmenu->height() > QApplication::desktop()->height() )
      pQPMmenu->popup(QPoint( g.x(), g.y() - pQPMmenu->height()));
  else
      pQPMmenu->popup(QPoint( g.x() + g.width(), g.y() + g.height()));
}

void TopLevel::paintEvent(QPaintEvent *pe) /*FOLD00*/
{
  QPainter p(this);
  int x = 1 + (12 - pQPpic->width()/2);
  int y = 1 + (12 - pQPpic->height()/2);
  p.drawPixmap(x , y, *pQPpic);
  p.end();
}

void TopLevel::newClipData()
{
    QString clipData = kapp->clipboard()->text();
    if(clipData != QSlast){
        QSlast = clipData.copy();
        if(clipData.isEmpty() || clipData.stripWhiteSpace().isEmpty()){ // If the string is null bug out
            return;
        }
        QString *data = new QString(clipData);
        data->detach();
        while(pQPMmenu->count() > 12){
            int id = pQPMmenu->idAt(2);
            pQIDclipData->remove(id);
            pQPMmenu->removeItemAt(2);

        }
        if(clipData.length() > 50){
            clipData.truncate(47);
            clipData.append("...");
        }
        long int id = pQPMmenu->insertItem(clipData.simplifyWhiteSpace(), -2, -1); // -2 means unique id, -1 means at end
        pQIDclipData->insert(id, data);
    }
}

void TopLevel::clickedMenu(int id)
{
    if(id == QUIT_ITEM)
        kapp->quit();
    else if (id == CONFIG_ITEM) {
      KKeyDialog::configureKeys( globalKeys );
    }
      
      pQTcheck->stop();
    QString *data = pQIDclipData->find(id);
    if(data != 0x0){
        kapp->clipboard()->setText(data->data());
        QSlast = data->copy();
    }
    
    else
        warning("Unable to find item: %d", id);
    pQTcheck->start(1000);
}

