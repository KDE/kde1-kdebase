/*
 * kbgndwm.cpp. Part of the KDE project.
 *
 * Copyright (C) 1997 Martin Jones
 *
 */

//----------------------------------------------------------------------------

#include <qpmcache.h>

#include <kwm.h>

#include "kbgndwm.h"

#include <X11/Xatom.h>

#include "kbgndwm.moc"

//----------------------------------------------------------------------------

Atom kwm_command;

//----------------------------------------------------------------------------

KBGndManager::KBGndManager( KWMModuleApplication * )
{
    QPixmapCache::clear();

    readSettings();

    QString group;

    desktops = new KBackground [ MAX_DESKTOPS ];

    for ( int i = 0; i < MAX_DESKTOPS; i++ )
    {
	group.sprintf( "Desktop%d", i+1 );
	desktops[i].readSettings( group );
    }

    current = KWM::currentDesktop() - 1;
    applyDesktop( current );
    
    kwm_command = XInternAtom( qt_xdisplay(), "KWM_COMMAND", False );
}

void KBGndManager::client_message( XClientMessageEvent *ev )
{
    if ( ev->message_type == kwm_command )
    {
	QString com = ev->data.b;
	if ( com == "kbgwm_reconfigure" )
	{
	    debug( "Got background reload event" );

	    QString oldName = desktops[current].getName();

	    QString group;
	    for ( int i = 0; i < MAX_DESKTOPS; i++ )
	    {
		group.sprintf( "Desktop%d", i+1 );
		desktops[i].readSettings( group );
	    }

	    if ( desktops[current].getName() != oldName )
		applyDesktop( current );
	}
    }
}

void KBGndManager::desktopChange( int d )
{
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

void KBGndManager::applyDesktop( int d )
{
    desktops[d].apply();
}

void KBGndManager::cacheDesktop()
{
    // cache current desktop
    if ( desktops[current].hasPixmap() )
    {
	QPixmap *pm = new QPixmap;
	*pm = *qApp->desktop()->backgroundPixmap();
	if ( !QPixmapCache::find( desktops[current].getName() ) )
	{
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
}

