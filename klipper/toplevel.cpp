/* -------------------------------------------------------------

   toplevel.cpp (part of Klipper - Cut & paste history for KDE)

   (C)  by

   Generated with the KDE Application Generator

 ------------------------------------------------------------- */


#include "toplevel.h"
#include <mykapp.h>
#include <kwm.h>
#include <qcursor.h>
#include <qintdict.h>
#include <qpainter.h>
#include "qmessagebox.h"
#include <qmenudata.h>
#include "kiconloader.h"
#include <kkeydialog.h>
#include <kconfig.h>

#define QUIT_ITEM   50
#define CONFIG_ITEM 60
#define POPUP_ITEM  70

/* XPM */
/* Drawn  by Andreas Thienemann for the K Desktop Environment */
/* See http://www.kde.org */
static const char*mouse[]={
/* columns rows colors chars-per-pixel */
"24 24 12 1",
"  c Gray0",
". c Gray19",
"X c #585858",
"o c #c05800",
"O c #ff8000",
"+ c #ffa858",
"@ c #808080",
"# c #a0a0a0",
"$ c #c3c3c3",
"% c gainsboro",
"& c Gray100",
"* c None",
/* pixels */
"*******          *******",
"****    %%%%%%%#    ****",
"***  OOO %$$$$# OOO  ***",
"*** O+    %###    +o  **",
"*** O+ &&      &$ +o  **",
"*** O+ &&&&&&&&&$ +o  **",
"*** O+ &&&&&&&&&$ +o  **",
"*** O+ &      &&$ +o  **",
"*** O+ &&&&&&&&&$ +o  **",
"*** O+ &     &&&$ +o  **",
"*** O+ &&&&&&&&&$ +o  **",
"*** O+ && &%.#&&$ +o  **",
"*** O+ &$.$.$&&&$ +o  **",
"*** O+ &@..%&&&&$ +o  **",
"*** O+ &XX.$&&&&$ +o  **",
"*** O+ & &#.%&&&$ +o  **",
"*** O+ $.&&@X&&&$ +o  **",
"*** O+ %$&&&#&&&$ +o  **",
"*** O+ $$$$$$$$$$ +o  **",
"*** O+            +o  **",
"*** O++++++++++++++o  **",
"***  oooooooooooooo   **",
"***                   **",
"****                  **"
};

TopLevel::TopLevel() /*FOLD00*/
  : KTMainWindow()
{
    KConfig *kc = kapp->getConfig();
    kc->setGroup("General");
    bPopupAtMouse = kc->readBoolEntry("PopupAtMousePosition", false);
  
    QSlast = "";
    pQPMmenu = new QPopupMenu(0x0, "main_menu");
    connect(pQPMmenu, SIGNAL(activated(int)),
            this, SLOT(clickedMenu(int)));

    pQPMsubMenu = new QPopupMenu(0x0, "sub_menu");
    connect(pQPMsubMenu, SIGNAL(activated(int)),
	    this, SLOT(clickedSubMenu(int)));
    pQPMsubMenu->setCheckable(true);
    pQPMsubMenu->insertItem(i18n("Shortcut..."), CONFIG_ITEM);
    pQPMsubMenu->insertItem(i18n("Popup at mouse position"), POPUP_ITEM);
    pQPMsubMenu->setItemChecked(POPUP_ITEM, bPopupAtMouse);
    pQPMsubMenu->insertSeparator();
    pQPMsubMenu->insertItem(i18n("Quit"), QUIT_ITEM);
    
    pQPMmenu->insertItem(i18n("Clipboard History"), pQPMsubMenu);
    pQPMmenu->insertSeparator();
    pQIDclipData = new QIntDict<QString>();
    pQIDclipData->setAutoDelete(TRUE);
    QSempty = i18n("<empty clipboard>");
    bClipEmpty = ((QString)kapp->clipboard()->text()).simplifyWhiteSpace().isEmpty();
    if(bClipEmpty)
        kapp->clipboard()->setText(QSempty);
    newClipData();
    pQTcheck = new QTimer(this, "timer");
    pQTcheck->start(1000, FALSE);
    connect(pQTcheck, SIGNAL(timeout()),
            this, SLOT(newClipData()));
    pQPpic = new QPixmap(mouse);

    globalKeys = new KGlobalAccel();
    globalKeys->insertItem(i18n("Select clipboard contents"),
				"select-clipboard", "CTRL+ALT+V");
    globalKeys->connectItem("select-clipboard", this, SLOT(showPopupMenu()));
    globalKeys->readSettings();
}

TopLevel::~TopLevel()
{
    delete pQTcheck;
    delete pQPMsubMenu;
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
	if (bClipEmpty) { // remove <clipboard empty> from popupmenu
	    if (*data != QSempty) {
	        bClipEmpty = false;
		pQPMmenu->removeItemAt(2);
	    }
	}
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
    pQTcheck->stop();
    QString *data = pQIDclipData->find(id);
    if(data != 0x0 && *data != QSempty){
        kapp->clipboard()->setText(data->data());
	QSlast = data->copy();
    }
  
    else
        warning("Unable to find item: %d", id);
    pQTcheck->start(1000);
}

void TopLevel::clickedSubMenu(int id)
{
    if(id == QUIT_ITEM) {
        kapp->quit();
    }
    else if (id == CONFIG_ITEM) {
        KKeyDialog::configureKeys(globalKeys);
    }
    else if (id == POPUP_ITEM) {
        bool isChecked = pQPMsubMenu->isItemChecked(POPUP_ITEM);
	pQPMsubMenu->setItemChecked(POPUP_ITEM, !isChecked);
	bPopupAtMouse = !isChecked;

	KConfig *kc = kapp->getConfig();
	kc->setGroup("General");
	kc->writeEntry("PopupAtMousePosition", bPopupAtMouse);
	kc->sync();
    }
}

void TopLevel::showPopupMenu()
{
  if (!bPopupAtMouse) {
      mousePressEvent(0L);
  }
  else {
    pQPMmenu->move(-1000,-1000);
    pQPMmenu->show();
    pQPMmenu->hide();
    QPoint g = QCursor::pos();
    if ( pQPMmenu->height() < g.y() )
      pQPMmenu->popup(QPoint( g.x(), g.y() - pQPMmenu->height()));
    else
      pQPMmenu->popup(QPoint(g.x(), g.y()));
  }
}
