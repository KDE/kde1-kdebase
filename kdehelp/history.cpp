//----------------------------------------------------------------------------
//
// Maintain a history of sites visited
//
// Copyright (c) 1997 The KDE project.
//

#include <config.h>
#include <qfile.h>
#include <qdstream.h>
#include "history.h"

KHistory::KHistory( const char *_file )
    : dict( 503 )
{
    dict.setAutoDelete( true );
    lifeTime = 30;

    filename = _file;
    loadHistory();
}

KHistory::~KHistory()
{
    saveHistory();
}

void KHistory::loadHistory()
{
    if ( filename.isEmpty() )
	return;

    dict.clear();
    QFile file( filename );

    if ( file.open( IO_ReadOnly ) )
    {
	QDate today = QDate::currentDate();
	QDataStream stream( &file );
	QString url = "";

	while ( !stream.eof() )
	{
	    stream >> url;

	    if ( url == "end" )
		break;

	    QDate *date = new QDate;
	    stream >> (*date);

	    if ( date->daysTo( today ) < lifeTime )
		dict.insert( url, date );
	    else
		delete date;
	}
    }
}

void KHistory::saveHistory()
{
    if ( filename.isEmpty() )
	return;

    QFile file( filename );

    if ( file.open( IO_WriteOnly ) )
    {
	QDataStream stream( &file );
	QDictIterator<QDate> it( dict );
	QString key;
	QDate *date;

	for ( ; it.currentKey(); ++it )
	{
	    key = it.currentKey();
	    stream << key;

	    date = it.current();
	    stream << *(date);
	}

	stream << QString( "end" );

	file.close();
    }
}

void KHistory::addURL( const char *_url )
{
    QDictIterator<QDate> it( dict );

    QDate *date = new QDate;
    *date = QDate::currentDate();

    dict.replace( _url, date );
}


