/*
 * kbgndwm.cpp. Part of the KDE project.
 *
 * Copyright (C) 1997 Martin Jones
 *               1998 Matej Koss
 *
 */

//----------------------------------------------------------------------------

#include <qpmcache.h>

#include <kwm.h>
#include <kprocess.h>

#include "kbgndwm.h"

#include "kbgndwm.moc"

//----------------------------------------------------------------------------

KBGndManager::KBGndManager( KWMModuleApplication * )
  : QWidget(0, "", 0)
{
  QPixmapCache::clear();

  docked = false;

  readSettings();

  desktops = new KBackground [ MAX_DESKTOPS ];

  for ( int i = 0; i < MAX_DESKTOPS; i++ )
    desktops[i].readSettings( i, oneDesktopMode, desktop );

  if ( oneDesktopMode )
    current = desktop;
  else
    current = KWM::currentDesktop() - 1;

  // popup menu for right mouse button
  popup_m = new QPopupMenu();
  CHECK_PTR( popup_m );
  popup_m->setCheckable( TRUE );

  popup_m->insertItem(i18n("Background Settings"), this, SLOT(settings()));
  popup_m->insertSeparator();
  o_id = popup_m->insertItem(i18n("Common Background"), this, SLOT(toggleOneDesktop()));
  popup_m->setItemChecked( o_id, oneDesktopMode );

  // setup icon
  QString pixdir = KApplication::kde_datadir();  
  pixmap = (pixdir + "/kdisplay/pics/logo.xpm").data();

  applyDesktop( current );

}


void KBGndManager::desktopChange( int d )
{
  if ( oneDesktopMode )
    {
      debug( "One desktop mode");
      return;
    }

  d = KWM::currentDesktop() - 1;

  debug( "Changing to desktop %d", d+1 );

  if ( desktops[current].getName() == desktops[d].getName() )
    {
      debug( "Desktops identical" );
      current = d;
      return;
    }

  if ( current != d )
    cacheDesktop();
      
  applyDesktop( d );

  current = d;
}



void KBGndManager::commandReceived( QString com )
{
  if ( com == "kbgwm_reconfigure" )
    {
      debug( "Got background reload event" );
      
      QString oldName = desktops[current].getName();
      
      KConfig config2(KApplication::kde_configdir() + "/kdisplayrc", 
		      KApplication::localconfigdir() + "/kdisplayrc");
      
      config2.setGroup( "Desktop Common" );
      oneDesktopMode = config2.readBoolEntry( "OneDesktopMode", false );
      popup_m->setItemChecked( o_id, oneDesktopMode );

      desktop = config2.readNumEntry( "DeskNum", 0 );
      if ( config2.readBoolEntry( "Docking", true ) )
	dock();
      else
	undock();

      for ( int i = 0; i < MAX_DESKTOPS; i++ )
	desktops[i].readSettings( i, oneDesktopMode, desktop );

      if ( oneDesktopMode )
	current = desktop;
      else
	current = KWM::currentDesktop() - 1;

      applyDesktop( current );

    }
}



void KBGndManager::applyDesktop( int d )
{
  desktops[d].apply();
  QString cmd = "kbgwm_change";
  KWM::sendKWMCommand( cmd );
}



void KBGndManager::cacheDesktop()
{
  // cache current desktop
  if ( desktops[current].hasPixmap() )
    {
      desktops[current].cancel();

      if ( qApp->desktop()->backgroundPixmap() == 0L )
	return;

      if ( !desktops[current].isApplied() )
	return;

      if ( !QPixmapCache::find( desktops[current].getName() ) )
	{
	  QPixmap *pm = new QPixmap;
	  *pm = *qApp->desktop()->backgroundPixmap();
	  if ( !QPixmapCache::insert( desktops[current].getName(), pm ) )
	    delete pm;
	}
    }
}


void KBGndManager::readSettings()
{
  KConfig *config = KApplication::getKApplication()->getConfig();

  config->setGroup( "General" );

  int cache = config->readNumEntry( "CacheSize", 1024 );

  if ( cache < 128 )
    cache = 128;
  if ( cache > 10240 )
    cache = 10240;

  QPixmapCache::setCacheLimit( cache );

  KConfig config2(KApplication::kde_configdir() + "/kdisplayrc", 
		  KApplication::localconfigdir() + "/kdisplayrc");

  config2.setGroup( "Desktop Common" );
  oneDesktopMode = config2.readBoolEntry( "OneDesktopMode", false );
  desktop = config2.readNumEntry( "DeskNum", 0 );

  if ( config2.readBoolEntry( "Docking", true ) )
    dock();
}


void KBGndManager::timeclick()
{
  desktops[current].randomize();
}


void KBGndManager::toggleOneDesktop()
{
  oneDesktopMode = !oneDesktopMode;
  desktop = KWM::currentDesktop() - 1;

  KConfig config2(KApplication::kde_configdir() + "/kdisplayrc", 
		  KApplication::localconfigdir() + "/kdisplayrc");
  
  config2.setGroup( "Desktop Common" );
  config2.writeEntry( "OneDesktopMode", oneDesktopMode );
  config2.writeEntry( "DeskNum", desktop );
  config2.sync();

  popup_m->setItemChecked( o_id, oneDesktopMode );

  for ( int i = 0; i < MAX_DESKTOPS; i++ )
    desktops[i].setOneDesktop( oneDesktopMode, desktop );

  if ( oneDesktopMode )
    current = desktop;
  else
    current = KWM::currentDesktop() - 1;

  applyDesktop( current );
}


void KBGndManager::paintEvent (QPaintEvent *e)
{
  (void) e;

  bitBlt(this, 0, 0, &pixmap);
}


void KBGndManager::mousePressEvent(QMouseEvent *e)
{
  if ( e->button() == LeftButton )
    timeclick();
  else if ( e->button() == RightButton ) {
    // open popup menu on right mouse button
    int x = e->x() + this->x();
    int y = e->y() + this->y();

    popup_m->popup(QPoint(x, y));
    popup_m->exec();
  }
}


void KBGndManager::settings()
{
  KShellProcess proc;
  proc << "kcmdisplay background";
  proc.start(KShellProcess::DontCare);
}


void KBGndManager::dock()
{
  if (!docked) {
    
    // prepare panel to accept this widget
    KWM::setDockWindow (this->winId());

    // that's all the space there is
    this->setFixedSize(24, 24);

    // finally dock the widget
    this->show();
    
    docked = true;
  } 
}

void KBGndManager::undock()
{
  if (docked) {

    // the widget's window has to be destroyed in order 
    // to undock from the panel. Simply using hide() is
    // not enough (seems to be necessary though).

    this->hide();

    this->destroy(true, true);

    // recreate window for further dockings
    this->create(0, true, false);

    docked = false;
  }
}
