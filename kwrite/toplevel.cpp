    /*
    Copyright (C) 1998 Jochen Wilhelmy
                       digisnap@cs.tu-berlin.de

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    */

#include <qkeycode.h>
#include <qmsgbox.h>

#include <kapp.h>
#include <kiconloader.h>
#include <kurl.h>

#include "toplevel.h"
#include "highlight.h"

// StatusBar field IDs
#define ID_LINE_COLUMN 1
#define ID_INS_OVR 2
#define ID_MODIFIED 3
#define ID_GENERAL 4

const toolUndo = 1;
const toolRedo = 2;

DocSaver docSaver;
QList<KWriteDoc> docList;

TopLevel::TopLevel (KWriteDoc *doc) : KTMainWindow ("KWrite") {

  setMinimumSize(180,120);

  recentFiles.setAutoDelete(TRUE);

  statusbarTimer = new QTimer(this);
  connect(statusbarTimer,SIGNAL(timeout()),this,SLOT(timeout()));

//  connect(mykapp,SIGNAL(kdisplayPaletteChanged()),this,SLOT(set_colors()));

  if (!doc) {
    doc = new KWriteDoc();
    docList.append(doc);
  }
  kWriteDoc = doc;

  setupEditWidget();
  setupMenuBar();
  setupToolBar();
  setupStatusBar();

  readConfig();

  recentPopup->clear();
  for (int z = 0 ; z < (int) recentFiles.count(); z++){
    recentPopup->insertItem(recentFiles.at(z));
  }

//  set_colors();

  hideToolBar = !hideToolBar;
  toggleToolBar();
  hideStatusBar = !hideStatusBar;
  toggleStatusBar();
  newCurPos();
  newStatus();
  newCaption();
  newUndo();

  KDNDDropZone *dropZone = new KDNDDropZone(this,DndURL);
  connect(dropZone,SIGNAL(dropAction(KDNDDropZone *)),
    this,SLOT(dropAction(KDNDDropZone *)));
}

TopLevel::~TopLevel() {

//  delete file;
//  delete edit;
//  delete help;
//  delete options;
//  delete recentpopup;
//  delete toolbar;
  if (kWrite->isLastView()) docList.remove(kWrite->doc());
}

void TopLevel::closeEvent(QCloseEvent *) {
  if (queryExit()) {
//    e->accept();
    delete this;
  }
}

bool TopLevel::queryExit() {
  int query;

  if (kWrite->isModified() && kWrite->isLastView()) {
    query = QMessageBox::information(this,
      i18n("Message"),
      i18n("This Document has been modified.\nWould you like to save it?"),
      i18n("Yes"),
      i18n("No"),
      i18n("Cancel"),
      0,2);

    switch (query) {
      case 0: //yes
        kWrite->save();
        if (kWrite->isModified()) return false;
        break;
      case 2: //cancel
        return false;
    }
  }
//  writeConfig();
  return true;
}

void TopLevel::loadURL(const char *url) {
  kWrite->loadURL(url);
}


void TopLevel::setupEditWidget() {


//  kWriteDoc = new KWriteDoc();
  kWrite = new KWrite(kWriteDoc, this);

  connect(kWrite,SIGNAL(newCurPos()),this,SLOT(newCurPos()));
  connect(kWrite,SIGNAL(newStatus()),this,SLOT(newStatus()));
  connect(kWrite,SIGNAL(statusMsg(const char *)),this,SLOT(statusMsg(const char *)));
  connect(kWrite,SIGNAL(newCaption()),this,SLOT(newCaption()));
  connect(kWrite,SIGNAL(newUndo()),this,SLOT(newUndo()));

  setView(kWrite,FALSE);
}


void TopLevel::setupMenuBar() {
  KMenuBar *menubar;

//  KStdAccel keys(kapp->getConfig());

  file =        new QPopupMenu();
  edit =        new QPopupMenu();
  options =     new QPopupMenu();
  help =        new QPopupMenu();
  recentPopup = new QPopupMenu();
//  colors =    new QPopupMenu();


  file->insertItem(i18n("Ne&w..."),kWrite,SLOT(newDoc()),CTRL+Key_N);
  file->insertItem(i18n("&Open..."),kWrite,SLOT(open()),CTRL+Key_O);
  file->insertItem(i18n("&Insert..."),kWrite,SLOT(insertFile()),CTRL+Key_I);
//  file->insertItem(i18n("Open Recen&t"), recentpopup);
//  connect(recentPopup,SIGNAL(activated(int)),SLOT(openRecent(int)));
  file->insertSeparator ();
  file->insertItem(i18n("&Save"),kWrite,SLOT(save()),CTRL+Key_S);
  file->insertItem(i18n("S&ave as..."),kWrite,SLOT(saveAs()));
  file->insertItem(i18n("&Close"),this,SLOT(closeWindow()),CTRL+Key_W);
  file->insertSeparator ();
//  file->insertItem(i18n("Open &URL..."),this,SLOT(file_open_url()));
//  file->insertItem(i18n("Save to U&RL..."),this,SLOT(file_save_url()));
//  file->insertSeparator ();
//  file->insertItem(i18n("&Print..."),this,SLOT(print()));
//  file->insertSeparator ();
//  file->insertItem (i18n("&Mail..."),this,SLOT(mail()));
//  file->insertSeparator ();
  file->insertItem (i18n("New &Window"),this,SLOT(newWindow()));
  file->insertItem (i18n("New &View"),this,SLOT(newView()));
//  file->insertSeparator ();
//  file->insertItem (i18n("E&xit"),this,SLOT(quitEditor()),CTRL+Key_Q);


  edit->insertItem(i18n("C&ut"),kWrite,SLOT(cut()),CTRL+Key_X);
  edit->insertItem(i18n("&Copy"),kWrite,SLOT(copy()),CTRL+Key_C);
  edit->insertItem(i18n("&Paste"),kWrite,SLOT(paste()),CTRL+Key_V);
  edit->insertSeparator();
  edit->insertItem(i18n("&Find..."),kWrite,SLOT(search()),CTRL+Key_F);
  edit->insertItem(i18n("&Replace..."),kWrite,SLOT(replace()),CTRL+Key_R);
  edit->insertItem(i18n("Find &Again"),kWrite,SLOT(searchAgain()),Key_F3);
  edit->insertItem(i18n("&Goto Line..."),kWrite,SLOT(gotoLine()),CTRL+Key_G);
  edit->insertSeparator();
  menuUndo = edit->insertItem(i18n("&Undo"),kWrite,SLOT(undo()),CTRL+Key_Z);
  menuRedo = edit->insertItem(i18n("&Redo"),kWrite,SLOT(redo()),CTRL+Key_Y);
  edit->insertSeparator();
//  edit->insertItem(i18n("Format..."),kWrite,SLOT(format()));
//  edit->insertSeparator();
  edit->insertItem(i18n("&Select All"),kWrite,SLOT(selectAll()));
  edit->insertItem(i18n("&Deselect All"),kWrite,SLOT(deselectAll()));
  edit->insertItem(i18n("&Invert Selection"),kWrite,SLOT(invertSelection()));
//  edit->insertSeparator();
//  edit->insertItem(i18n("Insert &Date"),this,SLOT(insertDate()));
//  edit->insertItem(i18n("Insert &Time"),this,SLOT(insertTime()));


  options->setCheckable(TRUE);
//  options->insertitem(i18n("&Font..."),this,SLOT(font()));
//  options->insertItem(i18n("Colors"),colors);
//  options->insertSeparator();
  options->insertItem(i18n("&Options..."),kWrite,SLOT(optDlg()));
  options->insertItem(i18n("&Highlight..."),this,SLOT(hlDlg()));
//  indentID = options->insertItem(i18n("Auto &Indent"),this,SLOT(toggle_indent_mode()));
  options->insertSeparator();
  menuVertical = options->insertItem(i18n("&Vertical Selections"),kWrite,SLOT(toggleVertical()),Key_F5);
  menuShowTB = options->insertItem(i18n("Show &Toolbar"),this,SLOT(toggleToolBar()));
  menuShowSB = options->insertItem(i18n("Show &Statusbar"),this,SLOT(toggleStatusBar()));
  options->insertSeparator();
  options->insertItem(i18n("Save Config"),this,SLOT(writeConfig()));
//  options->insertSeparator();
//  options->insertItem(i18n("Save Options"),this,SLOT(save_options()));


/*
  colors->insertItem(i18n("&Foreground Color"),
                     this, SLOT(set_foreground_color()));
  colors->insertItem(i18n("&Background Color"),
                     this, SLOT(set_background_color()));
*/

  help = kapp->getHelpMenu(true,
    "KWrite 0.92\n\nCopyright 1998\nJochen Wilhelmy\ndigisnap@cs.tu-berlin.de");

//  help->insertItem (i18n("&Help..."),this,SLOT(helpSelected()));
//  help->insertSeparator();
//  help->insertItem (i18n("&About..."),this,SLOT(about()));


  menubar = menuBar();//new KMenuBar(this,"menubar");
  menubar->insertItem(i18n("&File"),file);
  menubar->insertItem(i18n("&Edit"),edit);
  menubar->insertItem(i18n("&Options"),options);
  menubar->insertSeparator();
  menubar->insertItem(i18n("&Help"),help);

//  setMenu(menubar);
}


void TopLevel::setupToolBar(){
  KToolBar *toolbar;

  toolbar = toolBar();//new KToolBar(this);

  KIconLoader *loader = kapp->getIconLoader();

  QPixmap pixmap;

  pixmap = loader->loadIcon("filenew2.xpm");
  toolbar->insertButton(pixmap,0,SIGNAL(clicked()),
    kWrite,SLOT(newDoc()),TRUE,i18n("New Document"));

  pixmap = loader->loadIcon("fileopen.xpm");
  toolbar->insertButton(pixmap,0,SIGNAL(clicked()),
    kWrite,SLOT(open()),TRUE,i18n("Open Document"));

  pixmap = loader->loadIcon("filefloppy.xpm");
  toolbar->insertButton(pixmap,0,SIGNAL(clicked()),
    kWrite,SLOT(save()),TRUE,i18n("Save Document"));

  toolbar->insertSeparator();

  pixmap = loader->loadIcon("editcut.xpm");
  toolbar->insertButton(pixmap,0,SIGNAL(clicked()),
    kWrite,SLOT(cut()),TRUE,i18n("Cut"));

  pixmap = loader->loadIcon("editcopy.xpm");
  toolbar->insertButton(pixmap,0,SIGNAL(clicked()),
    kWrite,SLOT(copy()),TRUE,i18n("Copy"));

  pixmap = loader->loadIcon("editpaste.xpm");
  toolbar->insertButton(pixmap,0,SIGNAL(clicked()),
    kWrite,SLOT(paste()),TRUE,i18n("Paste"));

  toolbar->insertSeparator();

  pixmap = loader->loadIcon("back.xpm");
  toolbar->insertButton(pixmap,toolUndo,SIGNAL(clicked()),
    kWrite,SLOT(undo()),TRUE,i18n("Undo"));

  pixmap = loader->loadIcon("forward.xpm");
  toolbar->insertButton(pixmap,toolRedo,SIGNAL(clicked()),
    kWrite,SLOT(redo()),TRUE,i18n("Redo"));
/*
  toolbar->insertSeparator();

  pixmap = loader->loadIcon("fileprint.xpm");
  toolbar->insertButton(pixmap,0,SIGNAL(clicked()),
    this,SLOT(print()),TRUE,i18n("Print Document"));

  pixmap = loader->loadIcon("send.xpm");
  toolbar->insertButton(pixmap, 0,
                      SIGNAL(clicked()), this,
                      SLOT(mail()), TRUE, i18n("Mail Document"));

*/
  toolbar->insertSeparator();
  pixmap = loader->loadIcon("help.xpm");
  toolbar->insertButton(pixmap,0,SIGNAL(clicked()),
    this,SLOT(helpSelected()),TRUE,i18n("Help"));

  toolbar->setBarPos(KToolBar::Top);
}

void TopLevel::setupStatusBar(){
    KStatusBar *statusbar;
    statusbar = statusBar();//new KStatusBar( this );
    statusbar->insertItem("Line:000000 Col: 000", ID_LINE_COLUMN);
    statusbar->insertItem("XXX", ID_INS_OVR);
    statusbar->insertItem("*", ID_MODIFIED);
    statusbar->insertItem("", ID_GENERAL);

    statusbar->setInsertOrder(KStatusBar::RightToLeft);
    statusbar->setAlignment(ID_INS_OVR,AlignCenter);

    //    statusbar->setInsertOrder(KStatusBar::LeftToRight);
    //    statusbar->setBorderWidth(1);

//    setStatusBar( statusbar );
}



void TopLevel::newWindow() {

  TopLevel *t = new TopLevel();
  t->show();
}

void TopLevel::newView() {

  TopLevel *t = new TopLevel(kWriteDoc);
  t->show();
}


void TopLevel::closeWindow() {
  close();
}


void TopLevel::quitEditor() {

//  writeConfig();
  kapp->quit();
}

void TopLevel::hlDlg() {
  QStrList types;

  types.append("Normal");
  types.append("C");
  types.append("C++");
  types.append("HTML");
  types.append("BASH");

  kWrite->setHighlight(HighlightDialog::getHighlight(types,this,SLOT(newHl(int))));
}

void TopLevel::newHl(int index) {
  Highlight *highlight;

  switch (index) {
    case 1:
      highlight = new CHighlight("C/C++ Highlight");
      break;
    case 2:
      highlight = new CppHighlight("C/C++ Highlight");
      break;
    case 3:
      highlight = new HtmlHighlight("HTML Highlight");
      break;
    case 4:
      highlight = new BashHighlight("BASH Highlight");
      break;
    default:
      highlight = new NoHighlight("No Highlight");
  }
  highlight->init();
printf("TopLevel::newHl()\n");
  ((HighlightDialog *) sender())->newHl(highlight);
}

void TopLevel::toggleToolBar() {

  options->setItemChecked(menuShowTB,hideToolBar);
  if (hideToolBar) {
    hideToolBar = FALSE;
    enableToolBar(KToolBar::Show);
    //changeItem(i18n("Hide &Tool Bar"),toolID);
  } else {
    hideToolBar = TRUE;
    enableToolBar(KToolBar::Hide);
//    options->changeItem(i18n("Show &Tool Bar"),toolID);
  }
}

void TopLevel::toggleStatusBar() {

  options->setItemChecked(menuShowSB,hideStatusBar);
  if (hideStatusBar) {
    hideStatusBar = FALSE;
    enableStatusBar(KStatusBar::Show);
//    options->changeItem(i18n("Hide &Status Bar"),statusID);
  } else {
    hideStatusBar = TRUE;
    enableStatusBar(KStatusBar::Hide);
//    options->changeItem(i18n("Show &Status Bar"),statusID);
  }
}


void TopLevel::helpSelected() {
  kapp->invokeHTMLHelp( "" , "" );
}

void TopLevel::newCurPos() {
  char s[64];

  sprintf(s,"%s: %d %s: %d",i18n("Line"),kWrite->currentLine() +1,
                            i18n("Col"),kWrite->currentColumn() +1);
  statusBar()->changeItem(s,ID_LINE_COLUMN);
}

void TopLevel::newStatus() {
  int config;

  config = kWrite->config();
  options->setItemChecked(menuVertical,config & cfVerticalSelect);
  statusBar()->changeItem(config & cfOvr ? "OVR" : "INS",ID_INS_OVR);
  statusBar()->changeItem(kWrite->isModified() ? "*" : "",ID_MODIFIED);
}

void TopLevel::statusMsg(const char *msg) {
  statusbarTimer->stop();
  statusBar()->changeItem(msg,ID_GENERAL);
  statusbarTimer->start(10000,true); //single shot
}

void TopLevel::timeout() {
  statusBar()->changeItem("",ID_GENERAL);
}

void TopLevel::newCaption() {
  setCaption(kWrite->fileName());
// printf("caption %s\n",kWrite->fileName());
}

void TopLevel::newUndo() {
  int state;

  state = kWrite->undoState();
  edit->setItemEnabled(menuUndo,state & 1);
  edit->setItemEnabled(menuRedo,state & 2);
  toolBar()->setItemEnabled(toolUndo,state & 1);
  toolBar()->setItemEnabled(toolRedo,state & 2);
}

void TopLevel::dropAction(KDNDDropZone *dropZone) {
  char *s;

  QStrList &list = dropZone->getURLList();
  for (s = list.first(); s != 0L; s = list.next()) {
    // Load the first file in this window
    if (s == list.getFirst() && !kWrite->isModified()) {
       loadURL(s);
    } else {
//      setGeneralStatusField(klocale->translate("New Window"));
      TopLevel *t = new TopLevel();
//       setGeneralStatusField(klocale->translate("New Window Created"));
//       QString n = s;
      t->loadURL(s);
//      setGeneralStatusField(klocale->translate("Load Command Done"));
      t->show ();
    }
  }
}

//config
void TopLevel::readConfig() {
  KConfig *config;
  int w, h;

  config = kapp->getConfig();

  config->setGroup("General Options");
  w = config->readNumEntry("Width",550);
  h = config->readNumEntry("Height",400);
  resize(w,h);

  hideToolBar = config->readNumEntry("HideToolBar");
  hideStatusBar = config->readNumEntry("HideStatusBar");

  kWrite->readConfig(config);
}

void TopLevel::writeConfig() {
  KConfig *config;

  config = kapp->getConfig();

  config->setGroup("General Options");
  config->writeEntry("Width",width());
  config->writeEntry("Height",height());
  config->writeEntry("HideToolBar",hideToolBar);
  config->writeEntry("HideStatusBar",hideStatusBar);

  kWrite->writeConfig(config);
}

// session restore
void TopLevel::readProperties(KConfig *config) {

  kWrite->readSessionConfig(config);
}

void TopLevel::restore(KConfig *config, int n) {
  const char *url;

  if (kWrite->isLastView()) {
    url = kWrite->fileName();
    if (url && *url) loadURL(url);
  }
  readPropertiesInternal(config,n);
  show();
}

void restore() {
  KConfig *config;
  int docs, windows, z;
  char buf[16];
  KWriteDoc *doc;
  TopLevel *t;

  config = kapp->getSessionConfig();
  if (!config) return;

  config->setGroup("Number");
  docs = config->readNumEntry("NumberOfDocuments",0);
  windows = config->readNumEntry("NumberOfWindows",0);

  for (z = 1; z <= docs; z++) {
     sprintf(buf,"Document%d",z);
     config->setGroup(buf);
     doc = new KWriteDoc();
     docList.append(doc);
     doc->readSessionConfig(config);
  }

  for (z = 1; z <= windows; z++) {
    sprintf(buf,"%d",z);
    config->setGroup(buf);
    t = new TopLevel(docList.at(config->readNumEntry("DocumentNumber") - 1));
    t->restore(config,z);
  }
}


//session close
void TopLevel::saveProperties(KConfig *config) {

  config->writeEntry("DocumentNumber",docList.find(kWrite->doc()) + 1);
  kWrite->writeSessionConfig(config);
  setUnsavedData(kWrite->isModified());
}

//DocSaver::DocSaver() : QObject() {}

void DocSaver::saveYourself() {
  KConfig *config;
  int z;
  char buf[16];
  KWriteDoc *doc;

  config = kapp->getSessionConfig();
  config->setGroup("Number");
  config->writeEntry("NumberOfDocuments",docList.count());

  for (z = 1; z <= (int) docList.count(); z++) {
     sprintf(buf,"Document%d",z);
     config->setGroup(buf);
     doc = docList.at(z - 1);
     doc->writeSessionConfig(config);
  }
}

int main(int argc, char** argv) {
  KApplication a(argc,argv);

  QObject::connect(kapp,SIGNAL(saveYourself()),&docSaver,SLOT(saveYourself()));
  docList.setAutoDelete(false);

  if (kapp->isRestored()) {
    restore();
//    RESTORE(TopLevel);
  } else {
    TopLevel *t = new TopLevel();
    if (argc > 1) t->loadURL(argv[1]);
    t->show();
  }
  return a.exec();
}
