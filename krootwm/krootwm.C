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

#include <qmsgbox.h>
#include <kfm.h>
#include <kprocess.h>
#include <ksimpleconfig.h>

void execute(const char* cmd){
  KShellProcess proc;
  proc << cmd;
  proc.start(KShellProcess::DontCare);
}

DlgLineEntry::DlgLineEntry( const char *_text, const char* _value, QWidget *parent )
        : QDialog( parent, 0L, true )
{
    setGeometry( x(), y(), 350, 110 );

    QLabel *label = new QLabel( _text , this );
    label->setGeometry( 10, 10, 330, 15 );

    edit = new KLined( this, 0L );
    
    edit->setGeometry( 10, 40, 330, 25 );
    connect( edit, SIGNAL(returnPressed()), SLOT(accept()) );

    QPushButton *ok;
    QPushButton *clear;
    QPushButton *cancel;
    ok = new QPushButton( klocale->translate("Ok"), this );
    ok->setGeometry( 10,70, 80,30 );
    connect( ok, SIGNAL(clicked()), SLOT(accept()) );

    clear = new QPushButton( klocale->translate("Clear"), this );
    clear->setGeometry( 135, 70, 80, 30 );
    connect( clear, SIGNAL(clicked()), SLOT(slotClear()) );

    cancel = new QPushButton( klocale->translate("Cancel"), this );
    cancel->setGeometry( 260, 70, 80, 30 );
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
    
    kwmmapp = kwmmapp_arg;

    // parse the configuration
    KConfig *kconfig = KApplication::getKApplication()->getConfig();

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
    menuNew->insertItem( klocale->translate("Folder") );
    connect( menuNew, SIGNAL( activated( int ) ), 
	     this, SLOT( slotNewFile( int ) ) );
    
    templatesList.append( QString( "Folder") );

    // Find the templates path
    QString configpath = getenv( "HOME" );
    configpath += "/.kde/share/config/kfmrc";
    KSimpleConfig config( configpath );
    config.setGroup( "Paths" );

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
		 strcmp( fi->fileName().data(), ".." ) != 0 )
	    {
		QString tmp = fi->fileName().data();
		templatesList.append( tmp );
		if ( tmp.right(7) == ".kdelnk" )
		    tmp.truncate( tmp.length() - 7 );
		menuNew->insertItem( tmp );
	    }
	    ++it;                               // goto next list element
	}
    }

    rmb = new QPopupMenu;
    rmb->setMouseTracking(TRUE);
    rmb->installEventFilter(this);
    rmb->insertItem(klocale->translate("New"), menuNew );
    rmb->insertSeparator();
    rmb->insertItem(klocale->translate("Help on desktop"), RMB_HELP);
    rmb->insertItem(klocale->translate("Execute command"), RMB_EXECUTE);
    rmb->insertItem(klocale->translate("Display properties"), RMB_DISPLAY_PROPERTIES);
    rmb->insertItem(klocale->translate("Refresh desktop"), RMB_REFRESH_DESKTOP);
    //CT 13mar98 deskUnclutter and deskCascade
    rmb->insertItem(klocale->translate("Unclutter windows"), RMB_UNCLUTTER_WINDOWS);
    rmb->insertItem(klocale->translate("Cascade windows"), RMB_CASCADE_WINDOWS);
    rmb->insertItem(klocale->translate("Arrange icons"), RMB_ARRANGE_ICONS);
    rmb->insertSeparator();
    rmb->insertItem(klocale->translate("Lock screen"), RMB_LOCK_SCREEN);
    rmb->insertItem(klocale->translate("Logout"), RMB_LOGOUT);
    connect(rmb, SIGNAL(activated(int)), this, SLOT(rmb_menu_activated(int)));

    mmb = new QPopupMenu;
    mmb->setMouseTracking(TRUE);
    mmb->installEventFilter(this);
    mmb->setCheckable(TRUE);
    connect(mmb, SIGNAL(activated(int)), this, SLOT(mmb_menu_activated(int)));

    QApplication::desktop()->installEventFilter(this);  

    kwmmapp->connectToKWM();
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
	  XAllowEvents(qt_xdisplay(), AsyncPointer, CurrentTime);
	  XUngrabPointer(qt_xdisplay(), CurrentTime);
	  XSync(qt_xdisplay(), False);
	  KWM::sendKWMCommand(QString("kpanel:go")+x+y);
	}
	else {
	  int x, y, dx, dy;
	  x = e->pos().x();
	  y = e->pos().y();
	  dx = dy = 0;
	  if (select_rectangle(x,y,dx,dy)){
	    KFM* kfm = new KFM; 
	    kfm->selectRootIcons(x, y, dx, dy,
				 (e->state() & ControlButton) == ControlButton);
	    delete kfm;
	  }
	}
	return TRUE;
      break;
      case MidButton:
	generateWindowlist(mmb);
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
  
  return True;
}

void KRootWm::generateWindowlist(QPopupMenu* p){
  p->clear();
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
      p->insertItem(QString("&")+KWM::getDesktopName(d), 1000+d);
      if (!active_window && d == cd)
	p->setItemChecked(1000+d, TRUE);
      for (i=0; i<nw;i++){
	if (
	    (KWM::desktop(callbacklist[i]) == d
	     && !KWM::isSticky(callbacklist[i])
	     )
	    || 
	    (d == cd && KWM::isSticky(callbacklist[i]))
	    ){
	  p->insertItem(KWM::miniIcon(callbacklist[i], 16, 16),
			QString("   ")+KWM::titleWithState(callbacklist[i]),i);
	  if (callbacklist[i] == active_window)
	    p->setItemChecked(i, TRUE);
	}
      }
      if (d < nd)
	p->insertSeparator();
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
	p->insertItem(KWM::miniIcon(callbacklist[i], 16, 16),
		      KWM::titleWithState(callbacklist[i]),i);
	if (callbacklist[i] == active_window)
	  p->setItemChecked(i, TRUE);
      }
    }
  }
}

void KRootWm::slotNewFile( int _id )
{
    if ( menuNew->text( _id ) == 0 )
	return;
    
    QString p = templatesList.at( _id );
    
    QString text = klocale->translate("New");
    text += " ";
    text += p.data();
    text += ":";
    const char *value = p.data();

    if ( strcmp( p.data(), "Folder" ) == 0 ) {
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
	    QString cmd( "kfmclient copy \"" );
	    cmd += templatePath + p.data();
	    cmd += "\" \"";
	    cmd += desktopPath.data();
	    if ( cmd.right( 1 ) != "/" )
	        cmd += "/";
	    cmd += name.data();
	    cmd += "\"";
	    execute( cmd );
	}
    }
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
