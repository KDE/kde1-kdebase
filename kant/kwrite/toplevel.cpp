/*
    Copyright (C) 1998, 1999 Jochen Wilhelmy
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
#include <qmessagebox.h>
#include <qtabdialog.h>

#include <kapp.h>
#include <kiconloader.h>
#include <kurl.h>
#include <kstdaccel.h>

#include "toplevel.h"
#include "highlight.h"

// StatusBar field IDs
#define ID_LINE_COLUMN 1
#define ID_INS_OVR 2
#define ID_MODIFIED 3
#define ID_GENERAL 4

const int toolUndo = 1;
const int toolRedo = 2;

//command categories
const int ctFileCommands = 10;
//file commands
const int cmNew             = 1;
const int cmOpen            = 2;
const int cmInsert          = 3;
const int cmSave            = 4;
const int cmSaveAs          = 5;
const int cmPrint           = 6;
const int cmNewWindow       = 7;
const int cmNewView         = 8;
const int cmClose           = 9;


QList<KWriteDoc> docList; //documents
HlManager hlManager; //highlight manager
KGuiCmdManager cmdMngr; //manager for user -> gui commands


TopLevel::TopLevel (KWriteDoc *doc, const char *path) : KTMainWindow("KWrite") {

  setMinimumSize(180,120);

  recentFiles.setAutoDelete(TRUE);

  statusbarTimer = new QTimer(this);
  connect(statusbarTimer,SIGNAL(timeout()),this,SLOT(timeout()));

//  connect(kapp,SIGNAL(kdisplayPaletteChanged()),this,SLOT(set_colors()));

  if (!doc) {
    doc = new KWriteDoc(&hlManager, path); //new doc with default path
    docList.append(doc);
  }
  setupEditWidget(doc);
  setupMenuBar();
  setupToolBar();
  setupStatusBar();

//  readConfig();
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

void TopLevel::init() {

  hideToolBar = !hideToolBar;
  toggleToolBar();
  hideStatusBar = !hideStatusBar;
  toggleStatusBar();
  showPath = !showPath;
  togglePath();
  newCurPos();
  newStatus();
//  newCaption();
  newUndo();

  show();
}

bool TopLevel::queryClose() {
  if (!kWrite->isLastView()) return true;
  return kWrite->canDiscard();
//  writeConfig();
}

bool TopLevel::queryExit() {
  writeConfig();

  return true;
}

void TopLevel::loadURL(const char *url, int flags) {
  kWrite->loadURL(url,flags);
}


void TopLevel::setupEditWidget(KWriteDoc *doc) {

  kWrite = new KWrite(doc,this);

  connect(kWrite,SIGNAL(newCurPos()),this,SLOT(newCurPos()));
  connect(kWrite,SIGNAL(newStatus()),this,SLOT(newStatus()));
  connect(kWrite,SIGNAL(statusMsg(const char *)),this,SLOT(statusMsg(const char *)));
  connect(kWrite,SIGNAL(fileChanged()),this,SLOT(newCaption()));
  connect(kWrite,SIGNAL(newUndo()),this,SLOT(newUndo()));

  setView(kWrite,FALSE);
}

//--- test
/*
class Filter : public QObject {
  protected:
    bool eventFilter(QObject *, QEvent *e) {
      if (e->type() == Event_Accel) {    // key press
        QKeyEvent *k = (QKeyEvent *) e;
        debug("Ate key press %d", k->key());
        return true;
      }
      return false;
    }
};
*/

//---
void TopLevel::setupMenuBar() {
  KMenuBar *menubar;
  KGuiCmdPopup *file, *bookmarks;
  QPopupMenu *help, *popup;
  int z;

  KGuiCmdDispatcher *dispatcher;
  dispatcher = new KGuiCmdDispatcher(this, &cmdMngr);
  dispatcher->connectCategory(ctCursorCommands, kWrite, SLOT(doCursorCommand(int)));
  dispatcher->connectCategory(ctEditCommands, kWrite, SLOT(doEditCommand(int)));
  dispatcher->connectCategory(ctBookmarkCommands, kWrite, SLOT(doBookmarkCommand(int)));
  dispatcher->connectCategory(ctStateCommands, kWrite, SLOT(doStateCommand(int)));

  file =        new KGuiCmdPopup(dispatcher);
  edit =        new KGuiCmdPopup(dispatcher);
  bookmarks =   new KGuiCmdPopup(dispatcher);
  options =     new KGuiCmdPopup(dispatcher);
  help =        new QPopupMenu();
  recentPopup = new QPopupMenu();

  file->addCommand(ctFileCommands, cmNew, kWrite, SLOT(newDoc()));
  file->addCommand(ctFileCommands, cmOpen, kWrite, SLOT(open()));
  file->addCommand(ctFileCommands, cmInsert, kWrite, SLOT(insertFile()));
  file->insertItem(i18n("Open &Recent"), recentPopup);
  connect(recentPopup, SIGNAL(activated(int)), SLOT(openRecent(int)));
  file->insertSeparator ();
  file->addCommand(ctFileCommands, cmSave, kWrite, SLOT(save()));
  file->addCommand(ctFileCommands, cmSaveAs, kWrite, SLOT(saveAs()));
  file->insertSeparator ();
  file->addCommand(ctFileCommands, cmPrint, kWrite, SLOT(print()));
  file->insertSeparator ();
  file->addCommand(ctFileCommands, cmNewWindow, this, SLOT(newWindow()));
  file->addCommand(ctFileCommands, cmNewView, this, SLOT(newView()));
  file->insertSeparator ();
  file->addCommand(ctFileCommands, cmClose, this, SLOT(closeWindow()));

/*
  file->insertItem(i18n("&New..."),kWrite,SLOT(newDoc()),keys.openNew());
  file->insertItem(i18n("&Open..."),kWrite,SLOT(open()),keys.open());
  file->insertItem(i18n("&Insert..."),kWrite,SLOT(insertFile()));
  file->insertItem(i18n("Open &Recent"), recentPopup);
  connect(recentPopup,SIGNAL(activated(int)),SLOT(openRecent(int)));
  file->insertSeparator ();
  file->insertItem(i18n("&Save"),kWrite,SLOT(save()),keys.save());
  file->insertItem(i18n("Save &As..."),kWrite,SLOT(saveAs()));
  file->insertSeparator ();
  file->insertItem(i18n("&Print..."), kWrite,SLOT(print()),keys.print());
  file->insertSeparator ();
//  file->insertItem (i18n("&Mail..."),this,SLOT(mail()));
//  file->insertSeparator ();
  file->insertItem (i18n("New &Window"),this,SLOT(newWindow()));
  file->insertItem (i18n("New &View"),this,SLOT(newView()));
  file->insertSeparator ();
  file->insertItem(i18n("&Close"),this,SLOT(closeWindow()),keys.close());
//  file->insertItem (i18n("E&xit"),this,SLOT(quitEditor()),keys.quit());
*/

  menuUndo = edit->addCommand(ctEditCommands, cmUndo, Icon("undo.xpm"));
  menuRedo = edit->addCommand(ctEditCommands, cmRedo, Icon("redo.xpm"));
  edit->insertSeparator();
  edit->addCommand(ctEditCommands, cmCut);
  edit->addCommand(ctEditCommands, cmCopy);
  edit->addCommand(ctEditCommands, cmPaste);
  edit->insertSeparator();
  edit->addCommand(ctFindCommands, cmFind, kWrite, SLOT(find()));
  edit->addCommand(ctFindCommands, cmReplace, kWrite, SLOT(replace()));
  edit->addCommand(ctFindCommands, cmFindAgain, kWrite, SLOT(findAgain()));
  edit->addCommand(ctFindCommands, cmGotoLine, kWrite, SLOT(gotoLine()));
  edit->insertSeparator();
  edit->addCommand(ctEditCommands, cmIndent);
  edit->addCommand(ctEditCommands, cmUnindent);
  edit->insertSeparator();
  edit->addCommand(ctEditCommands, cmSelectAll);
  edit->addCommand(ctEditCommands, cmDeselectAll);
  edit->addCommand(ctEditCommands, cmInvertSelection);

/*
  menuUndo = edit->insertItem(i18n("U&ndo"),kWrite,SLOT(undo()),keys.undo());
  menuRedo = edit->insertItem(i18n("R&edo"),kWrite,SLOT(redo()),CTRL+Key_Y);
  edit->insertSeparator();
  edit->insertItem(i18n("C&ut"),kWrite,SLOT(cut()), keys.cut());
  edit->insertItem(i18n("&Copy"),kWrite,SLOT(copy()), keys.copy());
  edit->insertItem(i18n("&Paste"),kWrite,SLOT(paste()), keys.paste());
  edit->insertSeparator();
  edit->insertItem(i18n("&Find..."),kWrite,SLOT(search()),keys.find());
  edit->insertItem(i18n("&Replace..."),kWrite,SLOT(replace()),keys.replace());
  edit->insertItem(i18n("Find &Again"),kWrite,SLOT(searchAgain()),Key_F3);
  edit->insertItem(i18n("&Goto Line..."),kWrite,SLOT(gotoLine()),CTRL+Key_G);
  edit->insertSeparator();
  edit->insertItem(i18n("&Indent"),kWrite,SLOT(indent()),CTRL+Key_I);
  edit->insertItem(i18n("Uninden&t"),kWrite,SLOT(unIndent()),CTRL+Key_U);
  edit->insertSeparator();
//  edit->insertItem(i18n("Format..."),kWrite,SLOT(format()));
//  edit->insertSeparator();
  edit->insertItem(i18n("&Select All"),kWrite,SLOT(selectAll()));
  edit->insertItem(i18n("&Deselect All"),kWrite,SLOT(deselectAll()));
  edit->insertItem(i18n("In&vert Selection"),kWrite,SLOT(invertSelection()));
//  edit->insertSeparator();
//  edit->insertItem(i18n("Insert &Date"),this,SLOT(insertDate()));
//  edit->insertItem(i18n("Insert &Time"),this,SLOT(insertTime()));
*/

//  bookmarks->insertItem(i18n("&Set Bookmark..."),kWrite,SLOT(setBookmark()),ALT+Key_S);
//  bookmarks->insertItem(i18n("&Add Bookmark"),kWrite,SLOT(addBookmark()));
//  bookmarks->insertItem(i18n("&Clear Bookmarks"),kWrite,SLOT(clearBookmarks()),ALT+Key_C);
  kWrite->installBMPopup(bookmarks);

  //highlight selector
  hlPopup = new QPopupMenu();
  hlPopup->setCheckable(true);
  for (z = 0; z < hlManager.highlights(); z++) {
    hlPopup->insertItem(i18n(hlManager.hlName(z)),z);
  }
  connect(hlPopup,SIGNAL(activated(int)),kWrite,SLOT(setHl(int)));

  // end of line selector
  eolPopup = new QPopupMenu();
  eolPopup->setCheckable(true);
  eolPopup->insertItem("Unix", eolUnix);
  eolPopup->insertItem("Macintosh", eolMacintosh);
  eolPopup->insertItem("Windows/Dos", eolDos);
  connect(eolPopup,SIGNAL(activated(int)),kWrite,SLOT(setEol(int)));

  options->setCheckable(TRUE);
  options->insertItem(i18n("Set Highlight"),hlPopup);
  connect(hlPopup,SIGNAL(aboutToShow()),this,SLOT(showHighlight()));
  options->insertItem(i18n("&Defaults..."),kWrite,SLOT(hlDef()));
  options->insertItem(i18n("&Highlight..."),kWrite,SLOT(hlDlg()));
//  indentID = options->insertItem(i18n("Auto &Indent"),this,SLOT(toggle_indent_mode()));
  options->insertSeparator();
  options->insertItem(i18n("&Options..."),kWrite,SLOT(optDlg()));
  options->insertItem(i18n("&Colors..."),kWrite,SLOT(colDlg()));
  options->insertItem(i18n("End Of Line"),eolPopup);
  connect(eolPopup,SIGNAL(aboutToShow()),this,SLOT(showEol()));
 options->insertItem(i18n("&Keys..."), this, SLOT(keyDlg()));
  options->insertSeparator();
  menuVertical = options->addCommand(ctStateCommands, cmToggleVertical);
    //Item(i18n("&Vertical Selections"),kWrite,SLOT(toggleVertical()),Key_F5);
  menuShowTB = options->insertItem(i18n("Show &Toolbar"),this,SLOT(toggleToolBar()));
  menuShowSB = options->insertItem(i18n("Show &Statusbar"),this,SLOT(toggleStatusBar()));
  menuShowPath = options->insertItem(i18n("Show &Path"),this,SLOT(togglePath()));
  options->insertItem(i18n("Save Config"),this,SLOT(writeConfig()));
//  options->insertSeparator();
//  options->insertItem(i18n("Save Options"),this,SLOT(save_options()));

  help = kapp->getHelpMenu(true,
    i18n("KWrite 0.9.0\n\nCopyright 1998, 1999\nJochen Wilhelmy\ndigisnap@cs.tu-berlin.de"));

  //right mouse button popup
  // how can you have key bindings for RMB popups ? doesn't work
  //A: Note that accelerators only work for QPopupMenu items that live in a
  //menu bar. For stand-alone popup menus, use an independent QAccel object.
  popup = new QPopupMenu();
  popup->insertItem(i18n("&Open..."),kWrite,SLOT(open())/*,CTRL+Key_O*/);

  popup->insertItem(i18n("&Save"),kWrite,SLOT(save())/*,CTRL+Key_S*/);
  popup->insertItem(i18n("S&ave as..."),kWrite,SLOT(saveAs()));
  popup->insertSeparator();
  popup->insertItem(i18n("C&ut"),kWrite,SLOT(cut())/*,CTRL+Key_X*/);
  popup->insertItem(i18n("&Copy"),kWrite,SLOT(copy())/*,CTRL+Key_C*/);
  popup->insertItem(i18n("&Paste"),kWrite,SLOT(paste())/*,CTRL+Key_V*/);
  kWrite->installRBPopup(popup);

  menubar = menuBar();
  menubar->insertItem(i18n("&File"),file);
  menubar->insertItem(i18n("&Edit"),edit);
  menubar->insertItem(i18n("&Bookmarks"),bookmarks);
  menubar->insertItem(i18n("&Options"),options);
  menubar->insertSeparator();
  menubar->insertItem(i18n("&Help"),help);

//  installEventFilter(dispatcher);
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

//  pixmap = loader->loadIcon("undo.xpm");
  toolbar->insertButton(Icon("undo.xpm"), toolUndo,SIGNAL(clicked()),
    kWrite,SLOT(undo()),TRUE,i18n("Undo"));

  pixmap = loader->loadIcon("redo.xpm");
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


void TopLevel::openRecent(int id) {
  if (kWrite->canDiscard()) kWrite->loadURL(recentPopup->text(id));
}

void TopLevel::newWindow() {

  TopLevel *t = new TopLevel(0L, kWrite->fileName());
  t->readConfig();
  t->init();
//  t->kWrite->doc()->inheritFileName(kWrite->doc());
}

void TopLevel::newView() {

  TopLevel *t = new TopLevel(kWrite->doc());
  t->readConfig();
  t->kWrite->copySettings(kWrite);
  t->init();
}


void TopLevel::closeWindow() {
  close();
}


void TopLevel::quitEditor() {

//  writeConfig();
  kapp->quit();
}

void TopLevel::toggleToolBar() {

  options->setItemChecked(menuShowTB,hideToolBar);
  if (hideToolBar) {
    hideToolBar = FALSE;
    enableToolBar(KToolBar::Show);
  } else {
    hideToolBar = TRUE;
    enableToolBar(KToolBar::Hide);
  }
}

void TopLevel::keyDlg() {
  QTabDialog *qtd = new QTabDialog(this, "tabdialog", true);

  qtd->setCaption(i18n("Configure Keys..."));

  // keys
  //this still lacks layout management, so the tabdialog does not
  //make it fit
  KGuiCmdConfigTab *keys = new KGuiCmdConfigTab(qtd, &cmdMngr);
  qtd->addTab(keys, i18n("Keys"));

  qtd->setOkButton(i18n("OK"));
  qtd->setCancelButton(i18n("Cancel"));

  if (qtd->exec()) {
    // keys
    cmdMngr.changeAccels();
    cmdMngr.writeConfig(kapp->getConfig());
  } else {
    // cancel keys
    cmdMngr.restoreAccels();
  }

  delete qtd;
/*
  QDialog *dlg;

  cmdMngr.saveAccels();
  dlg = new KGuiCmdConfig(&cmdMngr, this);
  if (dlg->exec() == QDialog::Accepted) {
    cmdMngr.changeAccels();
    cmdMngr.writeConfig(kapp->getConfig());
  } else cmdMngr.restoreAccels();
  delete dlg;*/
}

void TopLevel::toggleStatusBar() {

  options->setItemChecked(menuShowSB, hideStatusBar);
  if (hideStatusBar) {
    hideStatusBar = FALSE;
    enableStatusBar(KStatusBar::Show);
  } else {
    hideStatusBar = TRUE;
    enableStatusBar(KStatusBar::Hide);
  }
}

void TopLevel::togglePath() {

  showPath = !showPath;
  options->setItemChecked(menuShowPath, showPath);
  newCaption();
}

void TopLevel::helpSelected() {
  kapp->invokeHTMLHelp( "" , "" );
}

void TopLevel::newCurPos() {
  char s[64];

  sprintf(s,"%1.20s: %d %1.20s: %d",i18n("Line"),kWrite->currentLine() +1,
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
  statusBar()->changeItem(msg, ID_GENERAL);
  statusbarTimer->start(10000, true); //single shot
}

void TopLevel::timeout() {
  statusBar()->changeItem("", ID_GENERAL);
}

void TopLevel::newCaption() {
  const char *caption, *fname;
  int z;

  if (kWrite->hasFileName()) {
    caption = kWrite->fileName();
    //set recent files popup menu
    z = (int) recentPopup->count();
    while (z > 0) {
      z--;
      if (!strcmp(caption, recentPopup->text(z))) recentPopup->removeItemAt(z);
    }
    recentPopup->insertItem(caption, 0, 0);
    if (recentPopup->count() > 5) recentPopup->removeItemAt(5);
    for (z = 0; z < 5; z++) recentPopup->setId(z, z);

    //set caption
    if (!showPath) {
      fname = strrchr(caption, '/');
      if (fname != 0L) caption = fname +1;
    }
    setCaption(caption);
  } else {
    setCaption(kapp->getCaption());
  }
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
      TopLevel *t = new TopLevel();
      t->readConfig();
      t->loadURL(s);
      t->init();
    }
  }
}


void TopLevel::showHighlight() {
  int hl = kWrite->getHl();

  for (int index = 0; index < (int) hlPopup->count(); index++)
    hlPopup->setItemChecked(index, hl == index);
}

void TopLevel::showEol() {
  int eol = kWrite->getEol();

  for (int index = 0; index < (int) eolPopup->count(); index++)
    eolPopup->setItemChecked(index, eol == index);
}

//common config
void TopLevel::readConfig(KConfig *config) {
  int z;
  char name[16];
  QString s;

  hideToolBar = config->readNumEntry("HideToolBar");
  hideStatusBar = config->readNumEntry("HideStatusBar");
  showPath = config->readNumEntry("ShowPath");

  for (z = 0; z < 5; z++) {
    sprintf(name, "Recent%d", z + 1);
    s = config->readEntry(name);
    if (!s.isEmpty()) recentPopup->insertItem(s);
  }
}

void TopLevel::writeConfig(KConfig *config) {
  int z;
  char name[16];

  config->writeEntry("HideToolBar",hideToolBar);
  config->writeEntry("HideStatusBar",hideStatusBar);
  config->writeEntry("ShowPath",showPath);

  for (z = 0; z < (int) recentPopup->count(); z++) {
    sprintf(name, "Recent%d", z + 1);
    config->writeEntry(name, recentPopup->text(z));
  }
}

//config file
void TopLevel::readConfig() {
  KConfig *config;
  int w, h;

  config = kapp->getConfig();

  config->setGroup("General Options");
  w = config->readNumEntry("Width",550);
  h = config->readNumEntry("Height",400);
  resize(w,h);

  readConfig(config);
//  hideToolBar = config->readNumEntry("HideToolBar");
//  hideStatusBar = config->readNumEntry("HideStatusBar");

  kWrite->readConfig(config);
  kWrite->doc()->readConfig(config);
}

void TopLevel::writeConfig() {
  KConfig *config;

  config = kapp->getConfig();

  config->setGroup("General Options");
  config->writeEntry("Width",width());
  config->writeEntry("Height",height());

  writeConfig(config);
//  config->writeEntry("HideToolBar",hideToolBar);
//  config->writeEntry("HideStatusBar",hideStatusBar);

  kWrite->writeConfig(config);
  kWrite->doc()->writeConfig(config);
}

// session management
void TopLevel::restore(KConfig *config, int n) {

  if (kWrite->isLastView() && kWrite->hasFileName()) { //in this case first view
    loadURL(kWrite->fileName(), lfNoAutoHl);
  }
  readPropertiesInternal(config, n);
  init();
//  show();
}

void TopLevel::readProperties(KConfig *config) {

  readConfig(config);
  kWrite->readSessionConfig(config);
}

void TopLevel::saveProperties(KConfig *config) {

  writeConfig(config);
  config->writeEntry("DocumentNumber",docList.find(kWrite->doc()) + 1);
  kWrite->writeSessionConfig(config);
  setUnsavedData(kWrite->isModified());
}

void TopLevel::saveData(KConfig *config) { //save documents
  int z;
  char buf[16];
  KWriteDoc *doc;

  config->setGroup("Number");
  config->writeEntry("NumberOfDocuments",docList.count());

  for (z = 1; z <= (int) docList.count(); z++) {
     sprintf(buf,"Document%d",z);
     config->setGroup(buf);
     doc = docList.at(z - 1);
     doc->writeSessionConfig(config);
  }
}

//restore session
void restore() {
  KConfig *config;
  int docs, windows, z;
  char buf[16];
  KWriteDoc *doc;
  TopLevel *t;

  config = kapp->getSessionConfig();
  if (!config) return;

  config->setGroup("Number");
  docs = config->readNumEntry("NumberOfDocuments");
  windows = config->readNumEntry("NumberOfWindows");

  for (z = 1; z <= docs; z++) {
     sprintf(buf,"Document%d",z);
     config->setGroup(buf);
     doc = new KWriteDoc(&hlManager);
     doc->readSessionConfig(config);
     docList.append(doc);
  }

  for (z = 1; z <= windows; z++) {
    sprintf(buf,"%d",z);
    config->setGroup(buf);
    t = new TopLevel(docList.at(config->readNumEntry("DocumentNumber") - 1));
    t->restore(config,z);
  }
}


int main(int argc, char **argv) {
  KGuiCmdApp a(argc, argv);
  //KApplication a(argc,argv);

  //list that contains all documents
  docList.setAutoDelete(false);

  //init commands
  KWrite::addCursorCommands(cmdMngr);

  cmdMngr.addCategory(ctFileCommands, "File Commands");
  cmdMngr.addCommand(cmNew,             "&New..."    );
  cmdMngr.addCommand(cmOpen,            "&Open..."   , CTRL+Key_O),
  cmdMngr.addCommand(cmInsert,          "&Insert..." );
  cmdMngr.addCommand(cmSave,            "&Save"      , CTRL+Key_S);
  cmdMngr.addCommand(cmSaveAs,          "Save &As...");
  cmdMngr.addCommand(cmPrint,           "&Print..."  , CTRL+Key_P);
  cmdMngr.addCommand(cmNewWindow,       "New &Window");
  cmdMngr.addCommand(cmNewView,         "New &View"  );
  cmdMngr.addCommand(cmClose,           "&Close"     , CTRL+Key_W, Key_Escape);
//  cmdMngr.addCommand(cmClose,           "&Quit"      );

  KWrite::addEditCommands(cmdMngr);
  KWrite::addFindCommands(cmdMngr);
  KWrite::addBookmarkCommands(cmdMngr);
  KWrite::addStateCommands(cmdMngr);

  cmdMngr.makeDefault();

  cmdMngr.readConfig(a.getConfig());

  if (kapp->isRestored()) {
    restore();
  } else {
    TopLevel *t = new TopLevel();
    t->readConfig();
    if (argc > 1) t->loadURL(argv[1],lfNewFile);
    t->init();
  }
  return a.exec();
}
