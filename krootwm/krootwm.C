/*
 * krootwm.C. Part of the KDE project.
 *
 * Copyright (C) 1997 Matthias Ettrich
 *
 * DlgLineEntry (c) 1997 Torben Weis, weis@kde.org
 *
 */

#include <qdir.h>

#include "krootwm.moc"
#include "version.h"

#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

#include <qmsgbox.h>
#include <kfm.h>
#include <kprocess.h>
#include <ksimpleconfig.h>
#include <kiconloader.h>
// --------- Sven's changes for macmode begin
#include <kmenubar.h>
// --------- Sven's changes for macmode end;


// Copied from kfm/utils.cpp. David
QString stringSqueeze( const char *str, unsigned int maxlen )
{
    QString s ( str );
    if (s.length() > maxlen) {
        int part = (maxlen-3)/2;
        return QString(s.left(part) + "..." + s.right(part));
    }
    else return s;
} 

void execute(const char* cmd){
  KShellProcess proc;
  proc << cmd;
  proc.start(KShellProcess::DontCare);
}

DlgLineEntry::DlgLineEntry( const char *_text, const char* _value, QWidget *parent )
        : QDialog( parent, 0L, true )
{
    setGeometry( x(), y(), 350, 100 );

    QLabel *label = new QLabel( _text , this );
    label->setGeometry( 10, 10, 330, 15 );

    edit = new KLined( this, 0L );

    edit->setGeometry( 10, 35, 330, 25 );
    connect( edit, SIGNAL(returnPressed()), SLOT(accept()) );

    QPushButton *ok;
    QPushButton *clear;
    QPushButton *cancel;
    ok = new QPushButton( klocale->translate("OK"), this );
    ok->setGeometry( 10,70, 80,25 );
    connect( ok, SIGNAL(clicked()), SLOT(accept()) );

    clear = new QPushButton( klocale->translate("Clear"), this );
    clear->setGeometry( 135, 70, 80, 25 );
    connect( clear, SIGNAL(clicked()), SLOT(slotClear()) );

    cancel = new QPushButton( klocale->translate("Cancel"), this );
    cancel->setGeometry( 260, 70, 80, 25 );
    connect( cancel, SIGNAL(clicked()), SLOT(reject()) );

    edit->setText( _value );
    edit->setFocus();
}

DlgLineEntry::~DlgLineEntry()
{
}

void DlgLineEntry::slotClear()
{
    edit->setText("");
}

KRootWm::KRootWm(KWMModuleApplication* kwmmapp_arg)
  :QObject(){

    
    wmakerMode = FALSE;
    
    // Torben
    bookmarkId = 10;
    bookmarkDict.setAutoDelete( true );

    kwmmapp = kwmmapp_arg;

    // parse the configuration
    KConfig *kconfig = KApplication::getKApplication()->getConfig();

    defaultPixmap = KApplication::getKApplication()->getIconLoader()->loadApplicationMiniIcon("mini-default.xpm", 16, 16);

    // --------- Sven's changes for macmode begin
    myMenuBar = 0;
    myMenuBarContainer = 0;

    kconfig->setGroup("KDE");//CT as Sven asked
    if (kconfig->readEntry("macStyle") == "on") //CT as Sven asked
      macMode = true;
    else
      macMode = false;
    // --------- Sven's changes for macmode end

    kconfig->setGroup("MouseButtons");
    QString s = kconfig->readEntry("Left", "Selection");

    kpanel_menu_on_left_button = (s == "Menu");

    if (s != "Selection" && s != "Menu"){
      kconfig->writeEntry("Left", "Selection");
      kconfig->sync();
    }

    XGCValues gv;
    unsigned long mask;

    XSelectInput(qt_xdisplay(), qt_xrootwin(),
 		 ButtonPressMask |
 		 ButtonReleaseMask |
 		 ButtonMotionMask
 		 );
    gv.function = GXxor;
    gv.line_width = 0;
    gv.foreground = WhitePixel(qt_xdisplay(), qt_xscreen())
      ^BlackPixel(qt_xdisplay(), qt_xscreen());
    gv.subwindow_mode = IncludeInferiors;
    mask = GCForeground | GCFunction | GCLineWidth
      | GCSubwindowMode;
	
    gc = XCreateGC(qt_xdisplay(), qt_xrootwin(), mask, &gv);

    // Torben
    // Creates the new menu
    menuNew = new QPopupMenu;
    CHECK_PTR( menuNew );

    connect( menuNew, SIGNAL( activated( int ) ),
	     this, SLOT( slotNewFile( int ) ) );

    // Find the templates path
    QString configpath = KApplication::localkdedir() + "/share/config/kfmrc";
    KSimpleConfig config( configpath, true ); /* read only ! */
    config.setGroup( "Paths" );

    connect(kwmmapp, SIGNAL(commandReceived(QString)),
		       this, SLOT(kwmCommandReceived(QString)));


    // Desktop Path
    desktopPath = QDir::homeDirPath() + "/Desktop/";
    desktopPath = config.readEntry( "Desktop", desktopPath);
    if ( desktopPath.right(1) != "/")
	desktopPath += "/";

    // Templates Path
    templatePath = desktopPath + "Templates/";
    templatePath = config.readEntry( "Templates" , templatePath);
    if ( templatePath.right(1) != "/")
	templatePath += "/";

    // Bookmark code ( Torben )
    bookmarks = new QPopupMenu();
    connect( bookmarks, SIGNAL( activated( int ) ),
	     SLOT( slotBookmarkSelected( int ) ) );
    updateBookmarkMenu();

    updateNewMenu (); // needs to be here because of templates path. David.

    // David create the popups here, so that buildMenubars can be called
    // several times
    rmb = new QPopupMenu;
    rmb->setMouseTracking(TRUE);
    rmb->installEventFilter(this);
    mmb = new QPopupMenu;
    mmb->setMouseTracking(TRUE);
    mmb->installEventFilter(this);
    mmb->setCheckable(TRUE);

    buildMenubars();
}

//CT 02Dec1998 - separate menubars building
void KRootWm::buildMenubars() {

  // --------- Sven's changes for macmode begin
    if (macMode)
    {
	myMenuBarContainer = new QWidget(0, 0, WStyle_Customize|WStyle_NoBorder);
	myMenuBar = new KMenuBar(myMenuBarContainer);

	myMenuBarContainer->setGeometry(0, qApp->desktop()->width()+10,100,40);
	myMenuBarContainer->show();
	myMenuBarContainer->hide();
	if (myMenuBar->menuBarPos() != KMenuBar::FloatingSystem) {
	    delete myMenuBarContainer;
	    myMenuBarContainer = 0;
	    myMenuBar = 0;
	}

     /*
     File    New    Bookmarks    Desktop     Windows    Help
     Exec,   temp-  bookmarks    disp.props  Window-    Help
     Lock,   lates  ...          refresh     list       About KDE
     Logout                      unclutter
                                 cascade
                                 arrange


      */

      if (myMenuBar)
      {
        file = new QPopupMenu;
        file->setMouseTracking(TRUE);
        file->installEventFilter(this);

        file->insertItem(klocale->translate("Execute command"), RMB_EXECUTE);
        file->insertSeparator();
        file->insertItem(klocale->translate("Lock screen"), RMB_LOCK_SCREEN);
        file->insertItem(klocale->translate("Logout"), RMB_LOGOUT);

        connect(file, SIGNAL(activated(int)), this, SLOT(rmb_menu_activated(int)));

        desk = new QPopupMenu;
        desk->setMouseTracking(TRUE);
        desk->installEventFilter(this);

        desk->insertItem(klocale->translate("Refresh desktop"), RMB_REFRESH_DESKTOP);
        desk->insertItem(klocale->translate("Unclutter windows"), RMB_UNCLUTTER_WINDOWS);
        desk->insertItem(klocale->translate("Cascade windows"), RMB_CASCADE_WINDOWS);
        desk->insertItem(klocale->translate("Arrange icons"), RMB_ARRANGE_ICONS);
        desk->insertItem(klocale->translate("Display properties"), RMB_DISPLAY_PROPERTIES);

        connect(desk, SIGNAL(activated(int)), this, SLOT(rmb_menu_activated(int)));

        help = new QPopupMenu;
        help->setMouseTracking(TRUE);
        help->installEventFilter(this);

        help->insertItem(klocale->translate("Help on desktop"), RMB_HELP);
        help->insertItem(klocale->translate("About KDE..."), kapp,
                         SLOT(aboutKDE()));

        connect(help, SIGNAL(activated(int)), this, SLOT(rmb_menu_activated(int)));
      }

    }

    rmb->clear();
    rmb->disconnect( this );
    mmb->clear();
    mmb->disconnect( this );

    // printf(myMenuBar ? "menuBar\n" : "no menuBar\n");
    if (!myMenuBar)
    {
      // This is because popupmenu cannot be submenu of two
      // QMenuData based classes QMenuBar or QPopupMenu :-(
      rmb->insertItem(klocale->translate("New"), menuNew );
      rmb->insertItem(klocale->translate("Bookmarks"), bookmarks );
      rmb->insertSeparator();
    }
    rmb->insertItem(klocale->translate("Help on desktop"), RMB_HELP);
    rmb->insertItem(klocale->translate("Execute command"), RMB_EXECUTE);
    rmb->insertItem(klocale->translate("Display properties"), RMB_DISPLAY_PROPERTIES);
    rmb->insertItem(klocale->translate("Refresh desktop"), RMB_REFRESH_DESKTOP);
    //CT 13mar98 deskUnclutter and deskCascade
    rmb->insertItem(klocale->translate("Unclutter windows"), RMB_UNCLUTTER_WINDOWS);
    rmb->insertItem(klocale->translate("Cascade windows"), RMB_CASCADE_WINDOWS);
    rmb->insertItem(klocale->translate("Arrange icons"), RMB_ARRANGE_ICONS);
    
    if (wmakerMode) {
	rmb->insertSeparator();
	rmb->insertItem(klocale->translate("Arrange Window Maker icons"), RMB_WMAKER_ARRANGE_ICONS);
	rmb->insertItem(klocale->translate("Show all"), RMB_WMAKER_SHOW_ALL);
	rmb->insertItem(klocale->translate("Hide other"), RMB_WMAKER_HIDE_OTHER);
	QPopupMenu* wmaker = new QPopupMenu;
	wmaker->insertItem(klocale->translate("Show info panel"), RMB_WMAKER_INFO);
	wmaker->insertItem(klocale->translate("Show legal panel"), RMB_WMAKER_LEGAL);
	wmaker->insertItem(klocale->translate("Restart"), RMB_WMAKER_RESTART);
	wmaker->insertSeparator();
	wmaker->insertItem(klocale->translate("Exit"), RMB_WMAKER_EXIT);
	rmb->insertItem(klocale->translate("Window Maker"), wmaker);
    }
    
    
    rmb->insertSeparator();
    rmb->insertItem(klocale->translate("Lock screen"), RMB_LOCK_SCREEN);
    rmb->insertItem(klocale->translate("Logout"), RMB_LOGOUT);
    connect(rmb, SIGNAL(activated(int)), this, SLOT(rmb_menu_activated(int)));

    connect(mmb, SIGNAL(activated(int)), this, SLOT(mmb_menu_activated(int)));
    connect(mmb, SIGNAL(aboutToShow()), this, SLOT(generateWindowlist()));

    if (myMenuBar)
    {
      myMenuBar->insertItem(klocale->translate("File"), file);
      myMenuBar->insertItem(klocale->translate("New"), menuNew);
      myMenuBar->insertItem(klocale->translate("Bookmarks"), bookmarks);
      myMenuBar->insertItem(klocale->translate("Desk"), desk);
      myMenuBar->insertItem(klocale->translate("Windows"), mmb);
      myMenuBar->insertItem(klocale->translate("Help"), help);
    }

    QApplication::desktop()->installEventFilter(this);

    kwmmapp->connectToKWM();

    if (myMenuBar && macMode)
    {
      KWM::setSticky(myMenuBar->winId(), true);
      connect(kwmmapp, SIGNAL(windowActivate (Window)), this,
              SLOT(slotFocusChanged(Window)));

      if (KWM::activeWindow() != None)
	  myMenuBar->lower();
    }


    // --------- Sven's changes for macmode end
  }

void KRootWm::kwmCommandReceived(QString com)
{

  //sent by KBookmarkManager::emitChanged()
  if (com == "krootwm:refreshBM")
      updateBookmarkMenu ();
  else if (com == "macStyleOn") {
      if (!macMode) {
	  kapp->getConfig()->reparseConfiguration();
	  macMode = true;
	  buildMenubars();
      }
  }
  else if (com == "macStyleOff") {
      if (macMode) {
	  delete myMenuBar;
	  myMenuBar = 0;
	  delete myMenuBarContainer;
	  myMenuBarContainer = 0;
	  macMode = false;
          updateBookmarkMenu();
          updateNewMenu ();
	  buildMenubars(); // will add New and Bookmark menus
      }
  }
  else if (com == "wm:wmaker") {
      wmakerMode = TRUE;
      buildMenubars();
  }
  else if (com == "wm:kwm") {
      wmakerMode = FALSE;
      buildMenubars();
  }

  else if (com== "krootwm:refreshNew")
    updateNewMenu ();
}

void KRootWm::updateBookmarkMenu (void)
{
  bookmarks->clear();
  bookmarks->insertItem( i18n( "Edit Bookmarks" ), 1 );
  bookmarks->insertSeparator();

  QString bdir( kapp->localkdedir().data() );
  bdir += "/share/apps/kfm/bookmarks";
  scanBookmarks( bookmarks, bdir );
}

void KRootWm::updateNewMenu (void)
{
    menuNew->clear();
    templatesList.clear();

    templatesList.append( QString( "Folder") );
    //menuNew->insertItem( klocale->translate("Folder") );
    menuNew->insertItem( kapp->getIconLoader()->loadMiniIcon("folder.xpm"), i18n("Folder") );
    QDir d( templatePath );
    const QFileInfoList *list = d.entryInfoList();
    if ( list == 0L )
        warning(klocale->translate("ERROR: Template does not exist '%s'"), templatePath.data());
    else
    {
	QFileInfoListIterator it( *list );      // create list iterator
	QFileInfo *fi;                          // pointer for traversing

	while ( ( fi = it.current() ) != 0L )
	{
	    if ( strcmp( fi->fileName().data(), "." ) != 0 &&
		 strcmp( fi->fileName().data(), ".." ) != 0 &&
                 !fi->isDir() && fi->isReadable())
	    {
		QString tmp = fi->fileName().data();
                KSimpleConfig config(templatePath + tmp.data(), true);
                config.setGroup( "KDE Desktop Entry" );
		templatesList.append( tmp );
		if ( tmp.right(7) == ".kdelnk" )
		    tmp.truncate( tmp.length() - 7 );
		// menuNew->insertItem( config.readEntry("Name", tmp ) );
                menuNew->insertItem( kapp->getIconLoader()->loadMiniIcon(config.readEntry("Icon",tmp)),
                                     config.readEntry("Name",tmp));

	    }
	    ++it;                               // goto next list element
	}
    }
}

//----------------------------------------------
//
// Bookmark code taken from KFM
// (c) Torben Weis, weis@kde.org
//
//----------------------------------------------

void KRootWm::scanBookmarks( QPopupMenu *_popup, const char * _path )
{
  struct stat buff;
  DIR *dp;
  struct dirent *ep;
  dp = opendir( _path );
  if ( dp == 0L ) {
      fprintf(stderr,strerror(errno)); // Can be "too many open files"...
      return;
  }

  // Loop thru all directory entries
  while ( ( ep = readdir( dp ) ) != 0L )
  {
    if ( strcmp( ep->d_name, "." ) != 0 && strcmp( ep->d_name, ".." ) != 0 )
    {
      // QString name = ep->d_name;	

      QString file = _path;
      file += "/";
      file += ep->d_name;
      stat( file.data(), &buff );
      if ( S_ISDIR( buff.st_mode ) )
      {
	QPixmap pix;
	QString f = kapp->kde_icondir().data();
	f += "/mini/folder.xpm";
	pix.load( f );

	QPopupMenu *pop = new QPopupMenu;
	connect( pop, SIGNAL( activated( int ) ),
	     SLOT( slotBookmarkSelected( int ) ) );

	_popup->insertItem( pix, ep->d_name, pop );
	scanBookmarks( pop, file );
      }
      else
      {
	KSimpleConfig cfg( file, true );
	cfg.setGroup( "KDE Desktop Entry" );
	QString type = cfg.readEntry( "Type" );	
	if ( type == "Link" )
	{
	  QString url = cfg.readEntry( "URL" );
	  QString icon = cfg.readEntry( "Icon", "unknown.xpm" );
	  QString miniicon = cfg.readEntry( "MiniIcon", icon );
	  if ( !url.isEmpty() && !miniicon.isEmpty() )
	  {
	    QPixmap pix;
	    QString name = cfg.readEntry( "Name" );
	    if ( name.isEmpty() )
	    {
	      name = ep->d_name;

	      int i = 0;
	      while ( ( i = name.find( "%%", i ) ) != -1 )
	      {
		name.replace( i, 2, "%");
		i++;
	      }

	      while ( ( i = name.find( "%2f" ) ) != -1 )
		name.replace( i, 3, "/");
	      while ( ( i = name.find( "%2F" ) ) != -1 )
		name.replace( i, 3, "/");
	
	      if ( name.length() > 7 && name.right( 7 ) == ".kdelnk" )
		name.truncate( name.length() - 7 );
	    }

	    QString f( kapp->localkdedir().data() );
	    f += "/share/icons/mini/";
	    f += miniicon;
	    if ( access( f, R_OK ) >= 0 )
	      pix.load( f );
	    else
	    {
	      f = kapp->kde_icondir().data();
	      f += "/mini/";
	      f += miniicon;
	      if ( access( f, R_OK ) >= 0 )
		pix.load( f );
	      else
	      {
		f = kapp->kde_icondir().data();
		f += "/mini/unknown.xpm";
		pix.load( f );
	      }
	    }
	    _popup->insertItem( pix, stringSqueeze(name,50), bookmarkId );
	    bookmarkDict.insert( bookmarkId++, new QString( url ) );
	  }
	}
      }
    }
  }
  closedir(dp);
}

bool KRootWm::eventFilter( QObject *obj, QEvent * ev){
  if (ev->type() == Event_MouseButtonPress
      || ev->type() == Event_MouseButtonDblClick){
    QMouseEvent *e = (QMouseEvent*) ev;
    if (obj == QApplication::desktop()){
      switch (e->button()){
      case LeftButton:
	if (kpanel_menu_on_left_button){
	  QString x,y;
	  x.setNum(e->pos().x());
	  y.setNum(e->pos().y());
	  while (x.length()<4)
	    x.prepend("0");
	  while (y.length()<4)
	    y.prepend("0");
	  //	  XAllowEvents(qt_xdisplay(), AsyncPointer, CurrentTime);
	  XUngrabPointer(qt_xdisplay(), CurrentTime);
	  XSync(qt_xdisplay(), False);
	  KWM::sendKWMCommand(QString("kpanel:go")+x+y);
	}
	else {
	  int x, y, dx, dy;
	  x = e->pos().x();
	  y = e->pos().y();
	  dx = dy = 0;
	  bool selected = select_rectangle(x,y,dx,dy);
	  KFM* kfm = new KFM;
	  kfm->selectRootIcons(x, y, dx, dy,
			       (e->state() & ControlButton) == ControlButton);
	  delete kfm;
          // --------- Sven's changes for macmode begin
            // Can I take focus from everybody here?
            if (!selected && macMode && myMenuBar)
            {
	KWM::activateInternal(None);
              myMenuBar->raise();
            }
          // --------- Sven's changes for macmode else
	}
	return TRUE;
      break;
      case MidButton:
	//generateWindowlist(mmb); sven disabled- using sig aboutToShow
	mmb->popup(e->pos());
	break;
      case RightButton:
	rmb->popup(e->pos());
	break;
      }
    }
  }
  return False;
}
	

void KRootWm::rmb_menu_activated(int item){
  switch (item) {
  case RMB_DISPLAY_PROPERTIES:
    execute("kcmdisplay");
    break;
  case RMB_ARRANGE_ICONS:
    {
      KFM* kfm = new KFM;
      kfm->sortDesktop();
      delete kfm;
    }
  break;
  case RMB_EXECUTE:
    KWM::sendKWMCommand("execute");
    break;
  case RMB_REFRESH_DESKTOP:
    {
      KFM* kfm = new KFM;
      kfm->refreshDesktop();
      KWM::refreshScreen();
      delete kfm;
    }
  break;
  //CT 13mar98 - deskUnclutter and deskCascade
  case RMB_UNCLUTTER_WINDOWS:
    KWM::sendKWMCommand("deskUnclutter");
    break;
  case RMB_CASCADE_WINDOWS:
    KWM::sendKWMCommand("deskCascade");
    break;
  case RMB_HELP:
    execute ("kdehelp");
    break;
  case RMB_LOCK_SCREEN:
    execute ("klock");
    break;
  case RMB_LOGOUT:
    KWM::logout();
    break;
    
  case RMB_WMAKER_INFO :
      KWM::sendKWMCommand("wmaker:lega");
      break;
  case RMB_WMAKER_LEGAL :
      KWM::sendKWMCommand("wmaker:info");
      break;
  case RMB_WMAKER_ARRANGE_ICONS :
      KWM::sendKWMCommand("wmaker:arrangeIcons");
      break;
  case RMB_WMAKER_SHOW_ALL :
      KWM::sendKWMCommand("wmaker:showAll");
      break;
  case RMB_WMAKER_HIDE_OTHER :
      KWM::sendKWMCommand("wmaker:hideOther");
      break;
  case RMB_WMAKER_RESTART :
      KWM::sendKWMCommand("wmaker:restart");
      break;
  case RMB_WMAKER_EXIT :
      KWM::sendKWMCommand("wmaker:exit");
      break;
  default:
    // nothing
    break;
  }
}

void KRootWm::mmb_menu_activated(int item){
  if (item>1000){
    KWM::switchToDesktop(item-1000);
  }
  else {
    Window w = callbacklist[item];
    delete [] callbacklist;
    KWM::activate(w);
  }

}


void KRootWm::draw_selection_rectangle(int x, int y, int dx, int dy){
   XDrawRectangle(qt_xdisplay(), qt_xrootwin(), gc, x, y, dx, dy);
   if (dx>2) dx-=2;
   if (dy>2) dy-=2;
   XDrawRectangle(qt_xdisplay(), qt_xrootwin(), gc, x+1, y+1, dx, dy);
}


bool KRootWm::select_rectangle(int &x, int &y, int &dx, int &dy){
  int cx, cy, rx, ry;
  int ox, oy;
  XEvent ev;

//   if (XGrabPointer(qt_xdisplay(), qt_xrootwin(), False,
// 		   ButtonPressMask | ButtonReleaseMask |
// 		   PointerMotionMask |
// 		   EnterWindowMask | LeaveWindowMask,
// 		   GrabModeAsync, GrabModeAsync, None,
// 		   arrowCursor.handle(), CurrentTime) == GrabSuccess){
  XChangeActivePointerGrab( qt_xdisplay(),
			    ButtonPressMask | ButtonReleaseMask |
			    PointerMotionMask ,
			    arrowCursor.handle(), 0);
  XGrabServer(qt_xdisplay());

  draw_selection_rectangle(x, y, dx, dy);

  ox = x;
  oy = y;

  cx = x;
  cy = y;

  for(;;) {
    XMaskEvent(qt_xdisplay(), ButtonPressMask|ButtonReleaseMask|
	       PointerMotionMask, &ev);

    if (ev.type == MotionNotify){
      rx = ev.xmotion.x_root;
      ry = ev.xmotion.y_root;
    }
    else
      break;
    if (rx == cx && ry == cy)
      continue;
    cx = rx;
    cy = ry;

    draw_selection_rectangle(x, y, dx, dy);

    if (cx > ox){	
      x = ox;
      dx = cx - x;
    }
    else {
      x = cx;
      dx = ox - x;
    }
    if (cy > oy){	
      y = oy;
      dy = cy - y;
    }
    else {
      y = cy;
      dy = oy - y;
    }

    draw_selection_rectangle(x, y, dx, dy);
    XFlush(qt_xdisplay());
  }

  draw_selection_rectangle(x, y, dx, dy);
  XFlush(qt_xdisplay());

  XUngrabServer(qt_xdisplay());
  XAllowEvents(qt_xdisplay(), AsyncPointer, CurrentTime);
  XSync(qt_xdisplay(), False);

  // --------- Sven's changes for macmode begin
  if (dx < 5 && dy < 5)
    return false;
  // --------- Sven's changes for macmode end
  return True;
}

void KRootWm::generateWindowlist(){ //sven changed this to slot
  mmb->clear();
  Window *w;
  int i = 0;
  int nw = kwmmapp->windows.count();
  callbacklist = new Window[nw];
  for (w = kwmmapp->windows.first(); w; w = kwmmapp->windows.next())
    callbacklist[i++]=*w;
  int d = 1;
  int nd = KWM::numberOfDesktops();
  int cd = KWM::currentDesktop();
  Window active_window = KWM::activeWindow();
  if (nd > 1){
    for (d=1; d<=nd; d++){
      mmb->insertItem(QString("&")+KWM::getDesktopName(d), 1000+d);
      if (!active_window && d == cd)
	mmb->setItemChecked(1000+d, TRUE);
      for (i=0; i<nw;i++){
	if (
	    (KWM::desktop(callbacklist[i]) == d
	     && !KWM::isSticky(callbacklist[i])
	     )
	    ||
	    (d == cd && KWM::isSticky(callbacklist[i]))
	    ){
	    QPixmap pm = KWM::miniIcon(callbacklist[i], 16, 16);
	    mmb->insertItem(pm.isNull()?defaultPixmap:pm,
			  QString("   ")+KWM::titleWithState(callbacklist[i]),i);
	  if (callbacklist[i] == active_window)
	    mmb->setItemChecked(i, TRUE);
	}
      }
      if (d < nd)
	mmb->insertSeparator();
    }
  }
  else {
    for (i=0; i<nw;i++){
      if (
	  (KWM::desktop(callbacklist[i]) == d
	   && !KWM::isSticky(callbacklist[i])
	   )
	  ||
	  (d == cd && KWM::isSticky(callbacklist[i]))
	  ){
	  QPixmap pm = KWM::miniIcon(callbacklist[i], 16, 16);
	  mmb->insertItem(pm.isNull()?defaultPixmap:pm,
			  KWM::titleWithState(callbacklist[i]),i);
	if (callbacklist[i] == active_window)
	  mmb->setItemChecked(i, TRUE);
      }
    }
  }
}

void KRootWm::slotNewFile( int _id )
{
    if ( menuNew->text( _id ) == 0 )
	return;

    QString p = templatesList.at( _id );
    QString tmp = p;
    tmp.detach();

    if ( strcmp( tmp.data(), "Folder" ) != 0 ) {
      QString x = templatePath + p.data();
      KSimpleConfig config(x, true);
      config.setGroup( "KDE Desktop Entry" );
      if ( tmp.right(7) == ".kdelnk" )
	tmp.truncate( tmp.length() - 7 );
      tmp = config.readEntry("Name", tmp);
    }

    QString text = klocale->translate("New");
    text += " ";
    text += tmp.data();
    text += ":";
    const char *value = p.data();

    if ( strcmp( tmp.data(), "Folder" ) == 0 ) {
	value = "";
	text = klocale->translate("New");
	text += " ";
	text += klocale->translate("Folder");
	text += ":";
    }

    DlgLineEntry l( text.data(), value, 0L );
    if ( l.exec() )
    {
	QString name = l.getText();
	if ( name.length() == 0 )
	    return;
	
	if ( strcmp( p.data(), "Folder" ) == 0 )
	{
	    QString u = desktopPath.data();
	    u.detach();
	    if ( u.right( 1 ) != "/" )
		u += "/";
	    u += name.data();
	    if ( mkdir( u, S_IRWXU ) == -1 )
	    {
		QString tmp;
		tmp.sprintf( "%s\n%s", klocale->translate("Could not create folder"), u.data() );
		QMessageBox::warning( 0L, "Error", tmp );
		return;
	    }	
	    else
	    {
		KFM* kfm = new KFM;
		kfm->refreshDesktop();
		delete kfm;
	    }
	}
	else
	{
          QString cmd( "cp \"" ); // using kfmclient copy here causes
          // problems with kfmclient openProperties coming next
          // (depending on machine's speed)
	    cmd += templatePath + p.data();
	    cmd += "\" \"";
	    cmd += desktopPath.data();
	    if ( cmd.right( 1 ) != "/" )
	        cmd += "/";
	    cmd += name.data();
	    cmd += "\"";
	    execute( cmd );

            sleep( 1 );
            
            cmd = "kfmclient openProperties \"" ;
	    cmd += desktopPath.data();
	    if ( cmd.right( 1 ) != "/" )
	        cmd += "/";
	    cmd += name.data();
	    cmd += "\"";
	    execute( cmd );

            KFM* kfm = new KFM;
            kfm->refreshDesktop();
            delete kfm;
	}
    }
}

void KRootWm::slotBookmarkSelected( int _id )
{
  if ( _id == 1 )
  {
    QString bdir( kapp->localkdedir().data() );
    bdir += "/share/apps/kfm/bookmarks";
    QString cmd( "kfmclient openURL \"" );
    cmd += bdir;
    cmd += "\"";
    execute( cmd );
    return;
  }

  QString* s = bookmarkDict[ _id ];
  if ( s == 0L )
  {
    warning( "Bug in KRootWm Bookmark code. Tell weis@kde.org" );
    return;
  }

  QString cmd( "kfmclient openURL \"" );
  cmd += s->data();
  cmd += "\"";
  execute( cmd );
}

void KRootWm::slotFocusChanged(Window w)
{
    static Window oldFocus = None;
  if (myMenuBar && w == None && w != oldFocus)
      myMenuBar->raise();

  oldFocus = w;
}

int main( int argc, char *argv[] )
{
  KWMModuleApplication a (argc, argv);
  if (argc > 1){
    if (QString("-version") == argv[1]){
      printf(KROOTWM_VERSION);
      printf("\n");
      printf(klocale->translate("Copyright (C) 1997 Matthias Ettrich (ettrich@kde.org)\n"));
      ::exit(0);
    }
    else {
      printf(klocale->translate("Usage:"));
      printf("%s [-version]\n", argv[0]);
    }
    ::exit(1);
  }
  fcntl(ConnectionNumber(qt_xdisplay()), F_SETFD, 1);
  KRootWm r(&a);
  return a.exec();
}
