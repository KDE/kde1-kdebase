// -*- C++ -*-

//
//  KDiskNavigator main file.
//
//  Copyright (C) 1998 Pietro Iglio
//  email:  iglio@kde.org
//

//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//
// PLEASE READ: if you are editing this file, probably you want to put your
//              hands on this source code. I suggest reading file
//              README.kdisknav-devel before hacking KDiskNavigator.
//
//
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#include "kpanel.h"

#include <qdstream.h>
#include <qtstream.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qapp.h>
#include <qfont.h>
#include <qdir.h>
#include <qpixmap.h>

#include <kapp.h>
#include <kiconloader.h>
#include <kfm.h>
#include <kmsgbox.h>

#include <ksimpleconfig.h>

#include "pmenu.h"

////////////////////////////////
//  DEBUG
////////////////////////////////

#ifdef DISKNAV_DEBUG
  int tot_activated = 0;
  int tot_deactivated = 0;
# define DEBUG_STMT(stmt)   stmt

#else
# define DEBUG_STMT(stmt)   ;
# undef ASSERT
# define ASSERT(stmt)       ;

#endif


/////////////////////////////////////////


#define DEFAULT_FOLDER_ICON        "folder.xpm"
#define DEFAULT_FILE_ICON          "document.xpm"
#define DEFAULT_EXECUTABLE_ICON    "exec.xpm"
#define LOCKED_FOLDER_ICON         "lockedfolder.xpm"
#define UNREADABLE_FOLDER_ICON     "mini-question.xpm"
#define MORE_ICON                  "blockdevice.xpm"
#define TOOMANYFILES_ICON          "mini-bomb.xpm"
#define OPTIONS_ICON               "application_settings.xpm"

#define KDISKNAV_SHARED_DIR       KApplication::kde_datadir() + "/kdisknav"
#define KDISKNAV_PERSONAL_DIR        "/.kde/share/apps/kdisknav"

#define GET_DEFAULT_FILE_ICON(fi)  \
   ((fi).isExecutable() ? DEFAULT_EXECUTABLE_ICON : DEFAULT_FILE_ICON)

#define MAX_FILENAME_LEN       256

extern void execute(const char*);
extern kPanel *the_panel;
PFileMenu* PFileMenu::root = 0L;
int PFileMenu::maxEntriesOnScreen = 256;
int PFileMenu::entryHeight = 0;

extern void copyFiles(QString, QString);

//////////////////////////////////////////////////////////////////////////////

// TEMPORARY SECTION BEGIN
// NOTE: This section is temporary here. It will be removed in future
// versions (kfm must create kdisknav dirs).

// #include <qmsgbox.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

// return true if the dir already exists
static bool testDir2( const char *_name )
{
    DIR *dp;
    QString c = getenv( "HOME" );
    c += _name;
    dp = opendir( c.data() );
    if ( dp == NULL ) {
      QString m(_name);
      //QMessageBox::information( 0, klocale->translate("KFM Information"),
      //		     klocale->translate("Creating directory:\n") + m );
	::mkdir( c.data(), S_IRWXU );
	return false;
    }
    else {
	closedir( dp );
	return true;
    }
}

// TEMPORARY SECTION END

//////////////////////////////////////////////////////////////////////////////
// isKdelnkFile("/usr/foo.kdelnk") -> true
// isKdelnkFile("/usr/foo") -> false
//
static bool
isKdelnkFile(const char* filename)
{
  int len = strlen(filename);

  return (len > 7 && filename[len - 7] == '.' &&
      strcmp(filename + len - 6, "kdelnk") == 0);
}

//////////////////////////////////////////////////////////////////////////////

static const char*
getFileInfo(const char* _path, const char* file_name, char* new_file,
            const QFileInfo* fi = 0)
{
  int len = strlen(file_name);
  strcpy(new_file, file_name);

  QString file(_path);

  file += "/";
  file += file_name;

  if (len > 7 && new_file[len - 7] == '.' &&
      strcmp(new_file + len - 6, "kdelnk") == 0) {
    new_file[len - 7] = 0;

    // QFileInfo fi(file);
    KSimpleConfig kconfig(file, true);
    kconfig.setGroup("KDE Desktop Entry");
    QString pixmap = kconfig.readEntry("MiniIcon");

    if (pixmap.isEmpty()) {
      QString big_pixmap_name = kconfig.readEntry("Icon");

      if (big_pixmap_name.isEmpty())
        return DEFAULT_EXECUTABLE_ICON;
      else
        return strdup((char*)big_pixmap_name.data());
    }
    else
      return strdup((char*) pixmap.data());
  }

  if (fi)
    return GET_DEFAULT_FILE_ICON(*fi);
  else
    return GET_DEFAULT_FILE_ICON(QFileInfo(file));
}


//////////////////////////////////////////////////////////////////////////////
//
// PFileMenu Constructors
//
//////////////////////////////////////////////////////////////////////////////

PFileMenu::PFileMenu(bool isRoot)
  : tail(0L), path(QString()), isClean(true), lastActivated(0)
{
  if (!testDir2(KDISKNAV_PERSONAL_DIR)) {
    // personal dir just created -> add some useful links
#ifdef OLD  // OLD STYLE (symlinks)
    ::symlink(QDir::homeDirPath(), (const char*)
	      QString(QDir::homeDirPath() +KDISKNAV_PERSONAL_DIR + "/Home"));
    ::symlink((const char*)(QDir::homeDirPath() + "/Desktop"), (const char*)
	      QString(QDir::homeDirPath() +KDISKNAV_PERSONAL_DIR + "/Desktop"));
#else  // NEW STYLE (URL files)
    QString kdisknav_personaldir = QDir::homeDirPath() +KDISKNAV_PERSONAL_DIR;

    copyFiles(KApplication::kde_datadir() + "/kpanel/default/Home.kdelnk",
	      kdisknav_personaldir + "/Home.kdelnk");
    copyFiles(KApplication::kde_datadir() + "/kpanel/default/Desktop.kdelnk",
	      kdisknav_personaldir + "/Desktop.kdelnk");

    /*
    FILE* fout = ::fopen(kdisknav_personaldir + "/Home.kdelnk", "w");

    if (fout != 0L) {
      ::fprintf(fout, "# KDE Config File\n");
      ::fprintf(fout, "[KDE Desktop Entry]\n");
      ::fprintf(fout, "MiniIcon=kfm_home.xpm\n");
      ::fprintf(fout, "Comment=Personal Files\n");
      ::fprintf(fout, "URL=file:$HOME\n");
      ::fprintf(fout, "Icon=kfm_home.xpm\n");
      ::fprintf(fout, "Type=Link\n");
      ::fclose(fout);
    }

    fout = ::fopen(kdisknav_personaldir + "/Desktop.kdelnk", "w");

    if (fout != 0L) {
      ::fprintf(fout, "# KDE Config File\n");
      ::fprintf(fout, "[KDE Desktop Entry]\n");
      ::fprintf(fout, "MiniIcon=desktop_settings.xpm\n");
      ::fprintf(fout, "Comment=Your desktop\n");
      ::fprintf(fout, "URL=file:$HOME/Desktop\n");
      ::fprintf(fout, "Icon=desktop_settings.xpm\n");
      ::fprintf(fout, "Type=Link\n");
      ::fclose(fout);
    }
    */

#endif
  }

  if (isRoot)
    buildRootMenu();
}

//////////////////////////////////////////////////////////////////////////////
PFileMenu::PFileMenu(QString& _path) : tail(0L), path(_path), isClean(true), lastActivated(0)
{
}

//////////////////////////////////////////////////////////////////////////////

PFileMenu::PFileMenu(const char* _path)
  : tail(0L), path(*new QString(_path)), isClean(true), lastActivated(0)
{
}

//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// PFileMenu Destructor
//
//////////////////////////////////////////////////////////////////////////////

PFileMenu::~PFileMenu()
{
  if (tail)
    delete tail;
}

//////////////////////////////////////////////////////////////////////////////

void PFileMenu::calculateMaxEntriesOnScreen(PMenuItem* menu)
{
  PFileMenu::maxEntriesOnScreen = menu->cmenu->maxEntriesOnScreen() -3;
  PFileMenu::entryHeight = menu->cmenu->entryHeight();
}

//////////////////////////////////////////////////////////////////////////////
// 'folder_name' is what appears in the menu bar
// if '_path' is specified, the effective folder is '_path'
// if '_path' is not specified, the effective folder is this->path + 'folder_name';
// Eg:
// newDirBrowserItem("myfolder") =   this->path + "/myfolder"
// newDirBrowserItem("myfolder", "/tmp/myfolder") =   "/tmp/myfolder"
//
PMenuItem*
PFileMenu::newDirBrowserItem(const QFileInfo* fi, bool useCurrentPath)
{
  QString newpath = useCurrentPath? this->path + "/" + fi->fileName()
                                  : QString(fi->filePath());

  PMenuItem* fileb =
    new PMenuItem(dirbrowser, fi->fileName() + "/", 0,
                  DEFAULT_FOLDER_ICON,
                  new PFileMenu(newpath), 0, 0, new myPopupMenu(this));

  CHECK_PTR(fileb);
  // The tooltip, in this case, is simply annoying
  fileb->comment = QString();
  return fileb;
}

//////////////////////////////////////////////////////////////////////////////
PMenuItem*
PFileMenu::newLinkItem(const QFileInfo* fi, bool useCurrentPath)
{
  KSimpleConfig kconfig(this->path + "/" + fi->fileName(), true);
  kconfig.setGroup("KDE Desktop Entry");

  if (kconfig.readEntry("Type") != "Link")
    return newFileItem(fi, useCurrentPath);   // <<---------

  QString url = kconfig.readEntry("URL");

  if (url.left(5) == "file:")
    url = url.right(url.length() - 5);   // remove "file:"

  struct stat urlstat;

  if (url.isEmpty() || stat(url.data(), &urlstat) != 0 || !S_ISDIR(urlstat.st_mode))
    return newFileItem(fi, useCurrentPath);   // <<---------

  QString pixmap_name = kconfig.readEntry("MiniIcon", DEFAULT_FOLDER_ICON);
  QString comment = kconfig.readEntry("Comment");
  QString name = fi->fileName().left( fi->fileName().length() - 7 );

#ifdef DISKNAV_DEBUG
  printf("this->path = [%s]\n", this->path.data());
  printf("name = [%s]\n", name.data());
  printf("url = [%s]\n", url.data());
  printf("pixmap_name = [%s]\n", pixmap_name.data());
  printf("comment = [%s]\n", comment.data());
#endif

  PMenuItem* fileb =
    new PMenuItem(dirbrowser, name + "/", 0,
                  pixmap_name,
                  new PFileMenu(url), 0, 0, new myPopupMenu(this),
		  false, QString(), comment);

  CHECK_PTR(fileb);
  return fileb;
}

//////////////////////////////////////////////////////////////////////////////
// if '_path' is specified, then create an entry '_path' + 'file_name';
// otherwise, create an entry 'this->_path' + 'file_name';
// The label is 'file_name';
//
// eg. newFileItem("emacs", "/usr/local/bin")
//
PMenuItem*
PFileMenu::newFileItem(const QFileInfo* fi, bool useCurrentPath)
{
  QString _path = (useCurrentPath ? this->path: fi->dirPath());

  char name[MAX_FILENAME_LEN];
  const char *pixmap = getFileInfo(_path, fi->fileName(), name, fi);

  PMenuItem* fileEntry = new PMenuItem(url, name, 0,
                                       pixmap,
                                       0, 0, 0, 0, false, _path);
  CHECK_PTR(fileEntry);
  fileEntry->setRealName(fi->fileName());
  // The tooltip, in this case, is simply annoying
  fileEntry->comment = QString();
  return fileEntry;
}

//////////////////////////////////////////////////////////////////////////////

void PFileMenu::deactivated(int _id)
{
  DEBUG_STMT(printf("%d: (%x) deactivated\n", _id, unsigned(this));)

  PMenuItem* item = searchItem(_id);
  PFileMenu* submenu = (PFileMenu*) item->sub_menu;

  ASSERT(searchItem(lastActivated));

  ASSERT(submenu->list.count() == submenu->cmenu->count());

  ASSERT(searchItem(lastActivated));

  if (lastActivated == submenu)  // VERIFY THIS!
    lastActivated = 0;

  if (!submenu->list.isEmpty()) {
    submenu->cmenu->clear();
    submenu->list.clear();
    submenu->isClean = true;
    submenu->lastActivated = 0L;
    // The destructor will take care of this
    // submenu->tail = 0L;
    submenu->finfos.clear();
    DEBUG_STMT(tot_deactivated++;)
  }

  DEBUG_STMT(printf("[%d %d]\n", tot_activated, tot_deactivated);)
  DEBUG_STMT(doSelfCheck();)
}

//////////////////////////////////////////////////////////////////////////////

void
PFileMenu::aboutToShow()
{
  if (this == PFileMenu::root)
    return;

  // HACK; TODO: understand why here we receive >1 events for a submenu;
#if QT_VERSION < 141
  //should not happen with qt-1.41 any more (matthias)
   if (this == lastActivated)
     return;
#endif

  DEBUG_STMT(printf("(%x) activated (clean=%d)\n", unsigned(this),
		this->isClean);)


  ASSERT(this->list.count() == this->cmenu->count());

  if (!this->isClean)
    return;
  else
    this->isClean = false;

  // Remove the unique entry -- a separator inserted by QPopupMenu::popup()
  // when it discovers that the menu is empty;
  this->cmenu->clear();

  if (this->parseDir(QDir(this->path)) == 0)
    this->add(new PMenuItem(label, klocale->translate("(empty folder)"),
                            0, "mini-eyes.xpm"));

  this->createMenu(this->parentItem->cmenu, the_panel);


#if QT_VERSION < 141
  // no longer necessary :-) (Matthias)
  this->fixMenuPosition();
#endif

  DEBUG_STMT(printf("[%d %d]\n", ++tot_activated, tot_deactivated);)
}

//////////////////////////////////////////////////////////////////////////////
//
int PFileMenu::parseTail()
{
    QFileInfoListIterator it(*tail);
    QFileInfo *fi;
    int numentries = 0;

    while ( (fi=it.current()) ) {
      if (numentries > PFileMenu::maxEntriesOnScreen) {
	QFileInfoListIterator* newit = new QFileInfoListIterator(it);
        this->addTailMenu(newit);
        break;
      }

      if (this->addFile(fi))
        numentries++;
      ++it;
    }
    return numentries;
}

//////////////////////////////////////////////////////////////////////////////
// Adds entries for the given folder to this PFileMenu.
// Return: number of added entries, -1 if the dir does not exist or is
// unreadable.
//
int PFileMenu::parseDir(QDir d, bool addOpenFolderEntry)
{
  if (tail)
    return parseTail();    // <<--------------

  d.setSorting(QDir::DirsFirst);

  if (the_panel->show_dot_files)
    d.setFilter(QDir::Hidden | d.filter());

  if (the_panel->ignore_case)
    d.setSorting(QDir::IgnoreCase | d.sorting());

  if (addOpenFolderEntry) {
    this->add( new PMenuItem(prog_com, klocale->translate("Open Folder"),
                           0, "folder_open.xpm", 0,
                           this, SLOT(openFolder()), 0, false, 0,
                           i18n("Open this folder (+Shift: Launch a terminal here)")));
    this->add( new PMenuItem(separator) );
  }

  const QFileInfoList *list;

  if (d.count() > the_panel->max_navigable_folder_entries) {
    list= 0;
    this->add( new PMenuItem(label, klocale->translate("(Too Many Files)"),
                               0, TOOMANYFILES_ICON));
    return -1;           // <<---------------
  }
  else
    list= d.entryInfoList();

  if (!list) {
    if (d.exists())
      this->add( new PMenuItem(label, klocale->translate("(Access Denied)"),
                               0, LOCKED_FOLDER_ICON));
    else
      this->add( new PMenuItem(label, klocale->translate("(Unreadable)"),
                               0, UNREADABLE_FOLDER_ICON));

    return -1;           // <<---------------
  }

  QFileInfoListIterator it( *list );


  QFileInfo *fi;
  int numentries = 0;
  int maxentries = PFileMenu::maxEntriesOnScreen -1;

  while ( (fi=it.current()) ) {
    if (numentries > maxentries) {
      this->copyTailFileInfo(it);
      break;
    }
    if (this->addFile(fi))
      numentries++;
    ++it;
  }

  return numentries;
}

//////////////////////////////////////////////////////////////////////////////
// Adds an entry for file 'fi' to this PFileMenu.
// Return false if the entry as been skipped (eg. '..')
//
bool PFileMenu::addFile(QFileInfo* fi, bool useDefaultPath)
{
  if( fi->fileName() == "." || fi->fileName() == ".." )
    return false;

  PMenuItem* item;

  if(fi->isDir())
    item = newDirBrowserItem(fi, useDefaultPath);
  else if (isKdelnkFile(fi->fileName()))
    item = newLinkItem(fi, useDefaultPath);
  else
    item = newFileItem(fi, useDefaultPath);

  this->add(item);

  return true;
}

//////////////////////////////////////////////////////////////////////////////
// Adds an entry for file 'fi' to this PFileMenu.
// Return false if the entry as been skipped (eg. '..')
//
bool PFileMenu::addFile(QString _path, bool useDefaultPath)
{
  QFileInfo fi(_path);

  // !! Don't worry about passing the address of a temporary object
  return addFile(&fi, useDefaultPath);
}
//////////////////////////////////////////////////////////////////////////////

void PFileMenu::addTailMenu(QFileInfoListIterator* _tail)
{
  PFileMenu* pfilemenu = new PFileMenu(this->path);
  pfilemenu->tail = _tail;

  this->add( new PMenuItem(separator) );

  myPopupMenu* mpm = new myPopupMenu;
  mpm->parentMenu = this;

  PMenuItem* item =
    new PMenuItem(dirbrowser, "More...",
                  0, MORE_ICON, pfilemenu, 0, 0,
                  mpm);

  this->add(item);
}

//////////////////////////////////////////////////////////////////////////////

void PFileMenu::copyTailFileInfo(QFileInfoListIterator& it)
{
  finfos.setAutoDelete(true);
  QFileInfo *fi;
  QFileInfo *newfi;

  while ( (fi=it.current()) ) {
    newfi = new QFileInfo(*fi);
    finfos.append(newfi);
    ++it;
  }

  QFileInfoListIterator* newit = new QFileInfoListIterator(finfos);
  addTailMenu(newit);
}

//////////////////////////////////////////////////////////////////////////////
//
void PFileMenu::insertRecentItem(EntryType type, const char* _path,
                                  const char* file_name)
{
  PMenuItem* item;
  char name[MAX_FILENAME_LEN];
  const char *pixmap;
  int index;

  if (!root)
     return;

  index = PFileMenu::root->cmenu->indexOf(the_panel->head_recent_id) + 1 +
              the_panel->recent_folders.count();

  if (type == dirbrowser) {
    PFileMenu* pfilemenu = new PFileMenu(_path);
    item = new PMenuItem(dirbrowser, QDir(_path).dirName() + "/",
                         0, DEFAULT_FOLDER_ICON, pfilemenu, 0, 0,
                         new myPopupMenu(root));

    item->sub_menu->createMenu(item->cmenu, the_panel );
    ((PFileMenu*)item->sub_menu)->id = item->getId();

    item->cmenu->id = item->getId();
    ((PFileMenu*)(item->sub_menu))->parentItem = item;

  } else {   // type == url

    pixmap = getFileInfo(_path, file_name, name);


    item = new PMenuItem(url, name, 0, pixmap, 0, 0, 0, 0, false, _path);

    item->setRealName(file_name);
    // root->cmenu->connectItem(item->getId(), item, SLOT(exec()) );
    index += the_panel->recent_files.count();
  }

  PFileMenu::root->cmenu->insertItem(item->pixmap, item->text_name, item->cmenu, item->getId(), index);

  if (type == url)
    root->cmenu->connectItem(item->getId(), item, SLOT(exec()));

  root->list.append(item);
}

//////////////////////////////////////////////////////////////////////////////


void PFileMenu::removeLessRecentItem(EntryType type, QStrList& recentlist)
{
  recentlist.removeFirst();

  int index = PFileMenu::root->cmenu->indexOf(the_panel->head_recent_id) + 2;

  if (type == url)
    index += the_panel->recent_folders.count();

  PFileMenu::root->cmenu->removeItemAt(index);
}

//////////////////////////////////////////////////////////////////////////////


void PFileMenu::updateRecentList(EntryType type, QString _path,
                              QStrList& recentlist, int max_size)
{
  if (!the_panel->show_recent_section)
    return;          // <<-----------------

  if (type == dirbrowser && the_panel->max_recent_folders_entries == 0)
    return;          // <<-----------------

  if (type == url && the_panel->max_recent_files_entries == 0)
    return;          // <<-----------------

  // check that "_path" is not already in

  for (const char* str = recentlist.first(); str; str = recentlist.next())
    if (!strcmp(str, _path))
      return;          // <<-----------------

  if (recentlist.count() >= (unsigned) max_size)
    removeLessRecentItem(type, recentlist);

  recentlist.append(_path);

  if (type == dirbrowser)
    insertRecentItem(dirbrowser, _path);
  else {
    QFileInfo fi(_path);
    insertRecentItem(url, fi.dirPath(), fi.fileName());
  }

  the_panel->writeOutRecentList(true);
}

//////////////////////////////////////////////////////////////////////////////

void PFileMenu::updateRecentFolders(QString _path)
{
  updateRecentList(dirbrowser, _path, the_panel->recent_folders,
                the_panel->max_recent_folders_entries);
}

void PFileMenu::updateRecentFiles(QString _path)
{
  updateRecentList(url, _path, the_panel->recent_files,
                the_panel->max_recent_files_entries);
}

//////////////////////////////////////////////////////////////////////////////
// Slot invoked when the "Open Folder" option is choosen from a dirbrowser
// menu.
//
void PFileMenu::openFolder()
{
  // Update the Recent section if the entry is not already in
  // the Shared or Personal section;
  if (!(this->parentItem && this->parentItem->cmenu &&
      this->parentItem->cmenu->parentMenu == PFileMenu::root))
    updateRecentFolders(this->path);

  QDir d(this->path);

  QString s = QString("\"") + d.canonicalPath() + "\"";

  if (myPopupMenu::keyStatus & ShiftButton) {
    KConfig *config = KApplication::getKApplication()->getConfig();
    config->setGroup( "kdisknav" );
    QString term = "kvt";
    term = config->readEntry( "Terminal", term );

    execute(QString("cd ") + s + "; " + term);
  }
  else
    execute(QString("kfmclient1 folder ") + s);
}

//////////////////////////////////////////////////////////////////////////////
// A root PFileMenu is the root of the menu hierarchy, and has a special
// layout; a root PFileMenu should be create before any other PFileMenu.
//
void PFileMenu::buildRootMenu()
{
    PFileMenu::root = this;
    bool isFirst = true;

    // Shared Section /////////////////////////////////////////////////////

    if (the_panel->show_shared_section) {
      this->add( new PMenuItem(label, i18n("Shared:"), 0, "shared.xpm",
		 0, 0, 0, 0, FALSE, QString(),
		 i18n("Entries shared among all users")) );
      this->add( new PMenuItem(separator) );

      this->path = QString(KDISKNAV_SHARED_DIR);
      QDir shareddir(this->path);
      this->parseDir(shareddir, false);
      isFirst = false;
    }


    // Personal Section /////////////////////////////////////////////////////

    if (the_panel->show_personal_section) {

      if (isFirst)
	isFirst = false;
      else
	this->add( new PMenuItem(separator) );

      this->add( new PMenuItem(label, i18n("Personal:"), 0, "personal.xpm",
		 0, 0, 0, 0, FALSE, QString(),
		 i18n("Personal entries")) );
      this->add( new PMenuItem(separator) );

      this->path = QString(QDir::homeDirPath() + KDISKNAV_PERSONAL_DIR);
      QDir personaldir(this->path);
      this->parseDir(personaldir, false);
    }

    // Recent Section /////////////////////////////////////////////////////

    if (the_panel->show_recent_section) {

      if (isFirst)
	isFirst = false;
      else
	this->add( new PMenuItem(separator) );

      PMenuItem* recentItem =
        new PMenuItem(label, i18n("Recent:"), 0, "mini-exclam.xpm",
		      0, 0, 0, 0, FALSE, QString(),
		      i18n("Recent entries"));
      the_panel->head_recent_id = recentItem->getId();
      this->add( recentItem );
      this->add( new PMenuItem(separator) );

      // Fix recent list if max entry options have been changed...

      unsigned count = the_panel->recent_folders.count();

      while (count && count > the_panel->max_recent_folders_entries) {
        the_panel->recent_folders.removeFirst();
        count--;
      }

      count = the_panel->recent_files.count();

      while (count && count > the_panel->max_recent_files_entries) {
        the_panel->recent_files.removeFirst();
        count--;
      }

      // ...done

      this->path = QString("");

      for (const char* entry = the_panel->recent_folders.first(); entry;
           entry = the_panel->recent_folders.next())
        this->addFile(entry, false);

      for (const char* entry = the_panel->recent_files.first(); entry;
           entry = the_panel->recent_files.next())
        this->addFile(entry, false);
    }

    // Options /////////////////////////////////////////////////////

    if (the_panel->show_option_entry ||
        (!the_panel->show_shared_section &&
         !the_panel->show_personal_section &&
         !the_panel->show_recent_section)) {

      if (isFirst)
	isFirst = false;
      else
	this->add( new PMenuItem(separator) );

      this->add(new PMenuItem(prog_com, i18n("Options..."), 0,
			      OPTIONS_ICON, 0, this, SLOT(optionsDlg()),
			      0, false, QString(), i18n("Customize Disk Navigator")));
    }

#ifdef DMALLOC
    this->add( new PMenuItem(prog_com, "END", 0, "kfm_refresh.xpm", 0,
                             this, SLOT(end())) );
#endif
}

//////////////////////////////////////////////////////////////////////////////

void PFileMenu::optionsDlg()
{
   KConfig *config = KApplication::getKApplication()->getConfig();
   config->setGroup("kdisknav");

   config->writeEntry("RecentFolders", the_panel->recent_folders);
   config->writeEntry("RecentFiles", the_panel->recent_files);

   config->sync();

   execute("kcmkpanel disknav");
}

/////////////////////////////////////////////////////////////////////
// This method is required due to a bug in implementation of QPopupMenu.
// After the aboutToShow() signal is emitted, in fact, a call to updateSize()
// is not followed by a recalculation of the menu position.
// Here we estimate the height of the menu and move it according to the
// desktop height.
// Note: once this bug is fixed in Qt, PFileMenu::entryHeight can be removed
// as well.
//
void PFileMenu::fixMenuPosition()
{
  int count_separators = 0;
  register PMenuItem *mi;
  QListIterator<PMenuItem> it(this->list);

  while ((mi = it.current())) {
    if (mi->getType() == separator)
        count_separators++;

    ++it;
  }

  int menu_height = (this->cmenu->count() - count_separators)
                     * PFileMenu::entryHeight + 2 * count_separators;

  int new_y = QApplication::desktop()->height() - menu_height;

  if (this->cmenu->y() > new_y)
    this->cmenu->move(this->cmenu->x(), new_y -2);
}

//////////////////////////////////////////////////////////////////////////////

#ifdef DISKNAV_DEBUG

void PMenu::dump()
{
  printf("PMenu::dump()\n");
  printf("[this = %x]", unsigned(this));

  for(PMenuItem* item = list.first(); item != 0; item = list.next() ) {
    printf("Dumping child of PMenu(%x):\n", unsigned(this));
    item->dump();
  }
}

void PFileMenu::dump()
{
  printf("PFileMenu::dump()\n");
  printf("[this = %x]", unsigned(this));
  printf("[lastActivated = %x]", unsigned(lastActivated));
  printf("[isClean = %d]", isClean);
  printf("[cmenu = %x]", unsigned(cmenu));
  printf("[id = %d]", id);
  printf("[tail = %x]", unsigned(tail));
  printf("[list.count() = %d]", list.count());

  printf("\n");

  int i;
  PMenuItem* item;

  for(item = list.first(), i = 0;
      item != 0; item = list.next(), i++) {
    printf("item n.%d (=%x) of %x", i, unsigned(item), unsigned(this));
    item->dump();
  }
}

void PMenuItem::dump()
{
  printf("[this = %x]", unsigned(this));
  printf("[id = %d]", id);
  printf("[entry_type = %d]", entry_type);
  printf("[text_name = %s]", text_name.data());
  printf("[cmenu = %x]", unsigned(cmenu));
  printf("[sub_menu = %x]", unsigned(sub_menu));

  printf("\n");

  if (entry_type == dirbrowser)
    sub_menu->dump();
}

void PFileMenu::doSelfCheck()
{
  if (tot_activated != tot_deactivated)
    return;

  // tot_activated == tot_deactivated => no Dirbrowser menu is active.

}

#endif

