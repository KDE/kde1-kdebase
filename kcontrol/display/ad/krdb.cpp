/****************************************************************************
**
** Copyright (C) 1998 by Mark Donohoe 
** This class is freely distributable under the GNU Public License.
**
*****************************************************************************/

#include <qpaintd.h>
#include <qdir.h>
#include <qdstream.h>
#include <qdatetm.h>
#include <qstring.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qtstream.h> 
#include <qbitmap.h>
#include <qpainter.h>
#include <qregexp.h> 

#include <stdlib.h>
#include <time.h>

#include <kapp.h>
#include <kprocess.h>


QString fontString( QFont rFont )
{
	QString aValue;
	
	aValue.sprintf( "-*-" );
	aValue += rFont.family();
	
	if ( rFont.bold() )
		aValue += "-bold";
	else
		aValue += "-medium";
	
	if ( rFont.italic() )
		aValue += "-i";
	else
		aValue += "-r";
		
	aValue += "-normal-*-*-";
		
	QString s;
	s.sprintf( "%d-*-*-*-*-", rFont.pointSize()*10 );
	aValue += s;
	
	switch ( rFont.charSet() ) {
		case QFont::Latin1:
			aValue += "iso8859-1";
			break;
		case QFont::AnyCharSet:
		default:
			aValue += "-*";
			break;
		case QFont::Latin2:
			aValue += "iso8859-2";
			break;
		case QFont::Latin3:
			aValue += "iso8859-3";
			break;
		case QFont::Latin4:
			aValue += "iso8859-4";
			break;
		case QFont::Latin5:
			aValue += "iso8859-5";
			break;
		case QFont::Latin6:
			aValue += "iso8859-6";
			break;
		case QFont::Latin7:
			aValue += "iso8859-7";
			break;
		case QFont::Latin8:
			aValue += "iso8859-8";
			break;
		case QFont::Latin9:
			aValue += "iso8859-9";
			break;
	}

  return aValue;
}

main( int argc, char ** argv ) 
{
	KApplication a( argc, argv );
	
	int contrast = 7;
	
	QString s;
	QString preproc;
	
	QColor col, backCol;
	
	QFont fnt( "helvetica", 12 );
	
	KConfig *config = kapp->getConfig();
	
	config->setGroup( "General" );

	col = config->readColorEntry( "foreground", &black );
	s.sprintf("#%02x%02x%02x\n", col.red(), col.green(), col.blue());
	s.prepend( "#define FOREGROUND " );
	preproc += s;
	 
	backCol = config->readColorEntry( "background", &lightGray );
	s.sprintf("#%02x%02x%02x\n", backCol.red(), backCol.green(), backCol.blue());
	s.prepend( "#define BACKGROUND " );
	preproc += s;

	col = config->readColorEntry( "selectBackground", &darkBlue);
	s.sprintf("#%02x%02x%02x\n", col.red(), col.green(), col.blue());
	s.prepend( "#define SELECT_BACKGROUND " );
	preproc += s;

	col = config->readColorEntry( "selectForeground", &white );
	s.sprintf("#%02x%02x%02x\n", col.red(), col.green(), col.blue());
	s.prepend( "#define SELECT_FOREGROUND " );
	preproc += s;

	col = config->readColorEntry( "windowBackground", &white );
	s.sprintf("#%02x%02x%02x\n", col.red(), col.green(), col.blue());
	s.prepend( "#define WINDOW_BACKGROUND " );
	preproc += s;

	col = config->readColorEntry( "windowForeground", &black );
	s.sprintf("#%02x%02x%02x\n", col.red(), col.green(), col.blue());
	s.prepend( "#define WINDOW_FOREGROUND " );
	preproc += s;
	
	fnt = config->readFontEntry( "font", new QFont( fnt ) );
	s = fontString( fnt );
	s += "\n";
	s.prepend( "#define FONT " );
	preproc += s;
	
	fnt = config->readFontEntry( "fixedFont", new QFont( fnt ) );
	s = fontString( fnt );
	s += "\n";
	s.prepend( "#define FIXED_FONT " );
	preproc += s;
	
	config->setGroup( "WM" );
	
	col = config->readColorEntry( "inactiveBackground", &darkGray);
	s.sprintf("#%02x%02x%02x\n", col.red(), col.green(), col.blue());
	s.prepend( "#define INACTIVE_BACKGROUND " );
	preproc += s;

	col = config->readColorEntry( "inactiveForeground", &lightGray );
	s.sprintf("#%02x%02x%02x\n", col.red(), col.green(), col.blue());
	s.prepend( "#define INACTIVE_FOREGROUND " );
	preproc += s;

	col = config->readColorEntry( "inactiveBlend", &lightGray );
	s.sprintf("#%02x%02x%02x\n", col.red(), col.green(), col.blue());
	s.prepend( "#define INACTIVE_BLEND " );
	preproc += s;

	col = config->readColorEntry( "activeBackground", &darkBlue );
	s.sprintf("#%02x%02x%02x\n", col.red(), col.green(), col.blue());
	s.prepend( "#define ACTIVE_BACKGROUND " );
	preproc += s;

	col = config->readColorEntry( "activeForeground", &white );
	s.sprintf("#%02x%02x%02x\n", col.red(), col.green(), col.blue());
	s.prepend( "#define ACTIVE_FOREGROUND " );
	preproc += s;

	col = config->readColorEntry( "activeBlend", &black );
	s.sprintf("#%02x%02x%02x\n", col.red(), col.green(), col.blue());
	s.prepend( "#define ACTIVE_BLEND " );
	preproc += s;
	
	fnt = config->readFontEntry( "titleFont", new QFont( fnt ) );
	s = fontString( fnt );
	s += "\n";
	s.prepend( "#define TITLE_FONT " );
	preproc += s;

	config->setGroup( "KDE" );

	contrast = config->readNumEntry( "contrast", 7 );
	
	int highlightVal=100+(2*contrast+4)*16/10;
	int lowlightVal=100+(2*contrast+4)*10;
	
	col = backCol.light(highlightVal);
	s.sprintf("#%02x%02x%02x\n", col.red(), col.green(), col.blue());
	s.prepend( "#define HIGHLIGHT " );
	preproc += s;
	
    col = backCol.dark(lowlightVal);
	s.sprintf("#%02x%02x%02x\n", col.red(), col.green(), col.blue());
	s.prepend( "#define LOWLIGHT " );
	preproc += s;
		
	QString adPath( kapp->kde_datadir() );
	adPath += "/kdisplay/app-defaults";
	
	QDir d;
	d.setPath( adPath );
	d.setFilter( QDir::Files );
	d.setSorting( QDir::Name );
	d.setNameFilter("*.ad");
	
	const QFileInfoList *sysList = d.entryInfoList();
	QFileInfoListIterator sysIt( *sysList );
		
	QFileInfo *fi;
	while ( ( fi = sysIt.current() ) ) {
		++sysIt;
	}
	sysIt.toFirst();
	
	
	adPath.sprintf( getenv( "HOME" ) );
	adPath += "/.kde/share/apps/kdisplay/app-defaults";
	
	d.setPath( adPath );
	d.setFilter( QDir::Files );
	d.setSorting( QDir::Name );
	d.setNameFilter("*.ad");
	
	QFileInfoList *userList = new QFileInfoList( *d.entryInfoList() );
	QFileInfoListIterator userIt( *userList );
	
	
	QString propString;
	
	long timestamp;
	::time( (long *) &timestamp ); 
	
	QString tmpFile;
	tmpFile.sprintf("/tmp/krdb.%ld", timestamp);
	
	QFile tmp( tmpFile );
	if ( tmp.open( IO_WriteOnly ) ) {
		tmp.writeBlock( preproc.data(), preproc.length() );

	while ( ( fi = sysIt.current() ) ) {
		if ( !userList->find( fi ) ) {
		QString fileName;
		fileName.sprintf("./%s", fi->filePath() );
		//fileName += fi->fileName();

		QFile f( fi->filePath() );

		if ( f.open(IO_ReadOnly) ) {    

			QTextStream t( &f ); 
			
    		while ( !t.eof() ) {
        		propString += t.readLine();       // line of text excluding '\n'
				propString +="\n";
    		}
    		f.close();
		}
		tmp.writeBlock( propString.data(), propString.length() );
		propString.resize(0);
		}
		++sysIt;
	}
	
	while ( ( fi = userIt.current() ) ) {
		QString fileName;
		fileName.sprintf("%s", fi->filePath() );
		//fileName += fi->fileName();

		QFile f( fi->filePath() );

		if ( f.open(IO_ReadOnly) ) {    

			QTextStream t( &f ); 
			
    		while ( !t.eof() ) {
        		propString += t.readLine();       // line of text excluding '\n'
				propString +="\n";
    		}
    		f.close();
		}
		tmp.writeBlock( propString.data(), propString.length() );
		propString.resize(0);
		++userIt;
	}
	
	tmp.close();
	}
	
	KProcess proc;
	
	proc.setExecutable("xrdb");
	proc << "-merge" << tmpFile.data();

	proc.start( KProcess::Block, KProcess::Stdin );
		
	d.setPath( "/tmp" );
		if ( d.exists() )
			d.remove( tmpFile );

}
