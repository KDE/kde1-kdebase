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

const int toolUndo = 1;
const int toolRedo = 2;

DocSaver docSaver;
QList<KWriteDoc> docList; //documents
HlManager hlManager; //highlight manager

TopLevel::TopLevel (KWriteDoc *doc) : KTMainWindow("KWrite") {

  setMinimumSize(180,120);

  recentFiles.setAutoDelete(TRUE);

  statusbarTimer = new QTimer(this);
  connect(statusbarTimer,SIGNAL(timeout()),this,SLOT(timeout()));

//  connect(kapp,SIGNAL(kdisplayPaletteChanged()),this,SLOT(set_colors()));

  setupEditWidget(doc);
  setupMenuBar();
  setupToolBar();
  setupStatusBar();

//  readConfig();
/*
  recentPopup->clear();
  for (int z = 0 ; z < (int) recentFiles.count(); z++){
    recentPopup->insertItem(recentFiles.at(z));
  }
*/
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
  newCurPos();
  newStatus();
  newCaption();
  newUndo();

  show();
}

void TopLevel::closeEvent(QCloseEvent *e) {
  if (queryExit()) {
    e->accept();
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

void TopLevel::loadURL(const char *url, int flags) {
  kWrite->loadURL(url,flags);
}


void TopLevel::setupEditWidget(KWriteDoc *doc) {

  if (!doc) {
    doc = new KWriteDoc(&hlManager);
    docList.append(doc);
  }

  kWrite = new KWrite(doc,this);

  connect(kWrite,SIGNAL(newCurPos()),this,SLOT(newCurPos()));
  connect(kWrite,SIGNAL(newStatus()),this,SLOT(newStatus()));
  connect(kWrite,SIGNAL(statusMsg(const char *)),this,SLOT(statusMsg(const char *)));
  connect(kWrite,SIGNAL(newCaption()),this,SLOT(newCaption()));
  connect(kWrite,SIGNAL(newUndo()),this,SLOT(newUndo()));

  setView(kWrite,FALSE);
}


void TopLevel::setupMenuBar() {
  KMenuBar *menubar;
  QPopupMenu *file, *help;
  KWBookPopup *bookmarks;
  int z;
//  KStdAccel keys(kapp->getConfig());

  file =        new QPopupMenu();
  edit =        new QPopupMenu();
  bookmarks =   new KWBookPopup();
  options =     new QPopupMenu();
  help =        new QPopupMenu();
  recentPopup = new QPopupMenu();


  file->insertItem(i18n("Ne&w..."),kWrite,SLOT(newDoc()),CTRL+Key_N);
  file->insertItem(i18n("&Open..."),kWrite,SLOT(open()),CTRL+Key_O);
  file->insertItem(i18n("&Insert..."),kWrite,SLOT(insertFile()));
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
  menuUndo = edit->insertItem(i18n("U&ndo"),kWrite,SLOT(undo()),CTRL+Key_Z);
  menuRedo = edit->insertItem(i18n("R&edo"),kWrite,SLOT(redo()),CTRL+Key_Y);
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

  bookmarks->insertItem(i18n("&Set Bookmark..."),kWrite,SLOT(setBookmark()),ALT+Key_S);
  bookmarks->insertItem(i18n("&Add Bookmark"),kWrite,SLOT(addBookmark()));
  bookmarks->insertItem(i18n("&Clear Bookmarks"),kWrite,SLOT(clearBookmarks()),ALT+Key_C);
  kWrite->installBMPopup(bookmarks);

  //highlight selector
  hlPopup = new QPopupMenu();
  hlPopup->setCheckable(true);
  for (z = 0; z < hlManager.highlights(); z++) {
    hlPopup->insertItem(i18n(hlManager.hlName(z)),z);
  }
  connect(hlPopup,SIGNAL(activated(int)),kWrite,SLOT(setHl(int)));

  options->setCheckable(TRUE);
  options->insertItem(i18n("Set Highlight"),hlPopup);
  connect(hlPopup,SIGNAL(aboutToShow()),this,SLOT(showHighlight()));
  options->insertItem(i18n("&Defaults..."),kWrite,SLOT(hlDef()));
  options->insertItem(i18n("&Highlight..."),kWrite,SLOT(hlDlg()));
//  indentID = options->insertItem(i18n("Auto &Indent"),this,SLOT(toggle_indent_mode()));
  options->insertSeparator();
  options->insertItem(i18n("&Options..."),kWrite,SLOT(optDlg()));
  options->insertItem(i18n("&Colors..."),kWrite,SLOT(colDlg()));
  options->insertSeparator();
  menuVertical = options->insertItem(i18n("&Vertical Selections"),kWrite,SLOT(toggleVertical()),Key_F5);
  menuShowTB = options->insertItem(i18n("Show &Toolbar"),this,SLOT(toggleToolBar()));
  menuShowSB = options->insertItem(i18n("Show &Statusbar"),this,SLOT(toggleStatusBar()));
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
    "KWrite 0.95\n\nCopyright 1998\nJochen Wilhelmy\ndigisnap@cs.tu-berlin.de");

//  help->insertItem (i18n("&Help..."),this,SLOT(helpSelected()));
//  help->insertSeparator();
//  help->insertItem (i18n("&About..."),this,SLOT(about()));


//  setMenu(menubar);

  //right mouse button popup
  popup = new QPopupMenu();
  popup->insertItem(i18n("&Open..."),kWrite,SLOT(open()),CTRL+Key_O);
  popup->insertItem(i18n("&Save"),kWrite,SLOT(save()),CTRL+Key_S);
  popup->insertItem(i18n("S&ave as..."),kWrite,SLOT(saveAs()));
  popup->insertSeparator();
  popup->insertItem(i18n("C&ut"),kWrite,SLOT(cut()),CTRL+Key_X);
  popup->insertItem(i18n("&Copy"),kWrite,SLOT(copy()),CTRL+Key_C);
  popup->insertItem(i18n("&Paste"),kWrite,SLOT(paste()),CTRL+Key_V);
  kWrite->installRBPopup(popup);

  menubar = menuBar();//new KMenuBar(this,"menubar");
  menubar->insertItem(i18n("&File"),file);
  menubar->insertItem(i18n("&Edit"),edit);
  menubar->insertItem(i18n("&Bookmarks"),bookmarks);
  menubar->insertItem(i18n("&Options"),options);
  menubar->insertSeparator();
  menubar->insertItem(i18n("&Help"),help);

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
  t->readConfig();
  t->init();
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
/*
void TopLevel::hlDlg() {
  QStrList types;

  types.append("Normal");
  types.append("C");
  types.append("C++");
  types.append("HTML");
  types.append("Bash");
  types.append("Modula 2");
  types.append("Ada");

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
      highlight = new BashHighlight("Bash Highlight");
      break;
    case 5:
      highlight = new ModulaHighlight("Modula Highlight");
      break;
    case 6:
      highlight = new AdaHighlight("Ada Highlight");
      break;
    default:
      highlight = new NoHighlight("No Highlight");
      index=0;
  }
  highlight->init();
printf("TopLevel::newHl()\n");
  ((HighlightDialog *) sender())->newHl(highlight);
}
*/
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
  statusBar()->changeItem(msg,ID_GENERAL);
  statusbarTimer->start(10000,true); //single shot
}

void TopLevel::timeout() {
  statusBar()->changeItem("",ID_GENERAL);
}

void TopLevel::newCaption() {
    if (kWrite->fileName())
	setCaption(kWrite->fileName());
    else
	setCaption(kapp->getCaption() );
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
  kWrite->doc()->readConfig(config);
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
  kWrite->doc()->writeConfig(config);
}

// session restore
void TopLevel::readProperties(KConfig *config) {

  kWrite->readSessionConfig(config);
}

void TopLevel::restore(KConfig *config, int n) {
  const char *url;

  if (kWrite->isLastView()) { //in this case first view
    url = kWrite->fileName();
    if (url && *url) loadURL(url,lfNoAutoHl);
  }
  readPropertiesInternal(config,n);
  init();
//  show();
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


//session close
void TopLevel::saveProperties(KConfig *config) {

  config->writeEntry("DocumentNumber",docList.find(kWrite->doc()) + 1);
  kWrite->writeSessionConfig(config);
  setUnsavedData(kWrite->isModified());
}


void TopLevel::showHighlight()
{
  int hl=kWrite->doc()->getHighlight();

  for (uint index=0; index<hlPopup->count(); index++)
    hlPopup->setItemChecked(index, hl == index);
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
/*
void initHighlight() { //setup highlight and default classes
  KConfig *config;
  DefItemStyle *p;
  QString s;
  QRgb col, selCol;
  char family[96];
  char charset[48];

  defItemStyleList.setAutoDelete(true);
  defItemStyleList.append(new DefItemStyle("Normal",black,white,false,false));
  defItemStyleList.append(new DefItemStyle("Keyword",black,white,true,false));
  defItemStyleList.append(new DefItemStyle("Decimal/Value",blue,cyan,false,false));
  defItemStyleList.append(new DefItemStyle("Base-N Integer",darkCyan,cyan,false,false));
  defItemStyleList.append(new DefItemStyle("Floating Point",darkMagenta,cyan,false,false));
  defItemStyleList.append(new DefItemStyle("Character",magenta,magenta,false,false));
  defItemStyleList.append(new DefItemStyle("String",red,red,false,false));
  defItemStyleList.append(new DefItemStyle("Comment",darkGray,gray,false,true));
  defItemStyleList.append(new DefItemStyle("Others",darkBlue,blue,false,false));

  config = kapp->getConfig();
  config->setGroup("Highlight Defaults");
  for (p = defItemStyleList.first(); p != 0L; p = defItemStyleList.next()) {
    s = config->readEntry(p->name);
    if (!s.isEmpty()) {
      sscanf(s,"%X,%X,%d,%d",&col,&selCol,&p->bold,&p->italic);
      p->col.setRgb(col);
      p->selCol.setRgb(selCol);
    }
  }
  s = config->readEntry("Font");
  if (!s.isEmpty()) {
    sscanf(s,"%95[^,],%d,%47[^,]",family,&itemFont.size,charset);
    itemFont.family = family;
    itemFont.charset = charset;
  }

  hlList.setAutoDelete(true);
  hlList.append(new Highlight("Normal"));
  hlList.append(new CHighlight());
  hlList.append(new CppHighlight());
  hlList.append(new HtmlHighlight());
  hlList.append(new BashHighlight());
  hlList.append(new ModulaHighlight());
  hlList.append(new AdaHighlight());
}
*/

int main(int argc, char** argv) {
  KApplication a(argc,argv);

  QObject::connect(kapp,SIGNAL(saveYourself()),&docSaver,SLOT(saveYourself()));
  docList.setAutoDelete(false);

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
