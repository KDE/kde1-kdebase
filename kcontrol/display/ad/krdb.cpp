/****************************************************************************
**
**
** KRDB - puts current KDE color and font scheme into preprocessor statements
** cats specially written application default files and uses xrdb -merge to
** write to RESOURCE_MANAGER. Thus it gives a  simple way to make non-KDE
** applications fit in with the desktop
**
** Copyright (C) 1998 by Mark Donohoe
** This application is freely distributable under the GNU Public License.
**
*****************************************************************************/

#include <qdir.h>
#include <qdstream.h>
#include <qdatetm.h>
#include <qstring.h>
#include <qtstream.h>

#include <stdlib.h>
#include <time.h>

#include <kapp.h>
#include <kprocess.h>

enum FontStyle { Normal, Bold, Italic, Fixed, Title };

QString fontString( QFont rFont, FontStyle style )
{
        if( style == Bold )
          rFont.setBold( true );

        if( style == Italic )
          rFont.setItalic( true );

        if( style == Fixed )
          rFont.setFixedPitch( true );

	QString aValue;
#if QT_VERSION > 140
	aValue = rFont.rawName();
	return aValue;
#endif

	QFontInfo fi( rFont );
	
	aValue.sprintf( "-*-" );
	aValue += fi.family();
	
	if ( fi.bold() )
		aValue += "-bold";
	else
		aValue += "-medium";
	
	if ( fi.italic() )
		aValue += "-i";
	else
		aValue += "-r";
		
	//aValue += "-normal-*-*-";
	//	
	//QString s;
	//s.sprintf( "%d-*-*-*-*-", fi.pointSize()*10 );
	//aValue += s;
	
	aValue += "-normal-*-";
		
	QString s;
	s.sprintf( "%d-*-*-*-*-*-", fi.pointSize() );
	aValue += s;
	
	
	switch ( fi.charSet() ) {
		case QFont::Latin1:
			aValue += "iso8859-1";
			break;
		case QFont::AnyCharSet:
		default:
			aValue += "*-*";
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
	s = fontString( fnt, Normal );
	s += "\n";
	s.prepend( "#define FONT " );
	preproc += s;
	
	fnt = config->readFontEntry( "font", new QFont( fnt ) );
	s = fontString( fnt, Bold );
	s += "\n";
	s.prepend( "#define BOLD_FONT " );
	preproc += s;

	fnt = config->readFontEntry( "font", new QFont( fnt ) );
	s = fontString( fnt, Italic );
	s += "\n";
	s.prepend( "#define ITALIC_FONT " );
	preproc += s;

	fnt = config->readFontEntry( "fixedFont", new QFont( "fixed", 12 ) );
	s = fontString( fnt, Fixed );
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
	s = fontString( fnt, Title );
	s += "\n";
	s.prepend( "#define TITLE_FONT " );
	preproc += s;

	s = "FONT, BOLD_FONT=BOLD, ITALIC_FONT=ITALIC";
	s += "\n";
        s.prepend( "#define FONTLIST ");
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
	
	//---------------------------------------------------------------
	
	QFileInfoList *sysList = 0;
	QFileInfoListIterator *sysIt = 0;
	QFileInfoList *userList = 0;
	QStrList *userNames = 0;
	QFileInfoListIterator *userIt = 0;
		
	QString adPath = kapp->kde_datadir().copy();
	adPath += "/kdisplay/app-defaults";
	QDir dSys;
	dSys.setPath( adPath );
	
	if ( dSys.exists() ) {
		dSys.setFilter( QDir::Files );
		dSys.setSorting( QDir::Name );
		dSys.setNameFilter("*.ad");
		sysList = new QFileInfoList( *dSys.entryInfoList() );
	}
	
	adPath.sprintf( KApplication::localkdedir().data() );
	adPath += "/share/apps/kdisplay/app-defaults";
	QDir dUser;
	dUser.setPath( adPath );
	
	if ( dUser.exists() ) {
		dUser.setFilter( QDir::Files );
		dUser.setSorting( QDir::Name );
		dUser.setNameFilter("*.ad");
		userList = new QFileInfoList( *dUser.entryInfoList() );
		userNames = new QStrList( *dUser.entryList() );
	}

	if ( !sysList && !userList ) {
		debug("No app-defaults files on system");
		exit(0);
	}
	
	if (sysList) sysIt = new QFileInfoListIterator( *sysList );
	if (userList) userIt = new QFileInfoListIterator( *userList );
	
	QString propString;

	time_t timestamp;
	::time( &timestamp );

	QString tmpFile;
	tmpFile.sprintf("/tmp/krdb.%d", timestamp);
	
	QFile tmp( tmpFile );
	if ( tmp.open( IO_WriteOnly ) ) {
			tmp.writeBlock( preproc.data(), preproc.length() );
	} else {
		debug("Couldn't open temp file");
		exit(0);
	}
	
	
//	debug("Creating file %s", tmpFile.data() );

	QFileInfo *fi;
	if ( sysList  ) {
		//debug("Found system list");
	
		while ( ( fi = sysIt->current() ) ) {
			int result = -1;
			if ( userList )
				result = userNames->find( fi->fileName() );
		
			if ( result != -1 ) {
				//debug("System ad's overridden by user ads.");
			} else {
				
				//debug("Concatenate %s",  fi->filePath() );

				QFile f( fi->filePath() );

				if ( f.open(IO_ReadOnly) ) {

					QTextStream t( &f );

    				while ( !t.eof() ) {
        				propString += t.readLine();
						propString +="\n";
    				}
    				f.close();
				}
				tmp.writeBlock( propString.data(), propString.length() );
				propString.resize(0);
			}
			++*sysIt;
		}
	}
	
	if ( userList ) {
		//debug("Found user list");
		while ( ( fi = userIt->current() ) ) {
			//debug("Concatenate %s",  fi->filePath() );

			QFile f( fi->filePath() );

			if ( f.open(IO_ReadOnly) ) {

				QTextStream t( &f );

    			while ( !t.eof() ) {
        			propString += t.readLine();
					propString +="\n";
    			}
    			f.close();
			}
			tmp.writeBlock( propString.data(), propString.length() );
			propString.resize(0);
			++*userIt;
		}
	}

	tmp.close();

	KProcess proc;

	proc.setExecutable("xrdb");
	proc << "-merge" << tmpFile.data();

	proc.start( KProcess::Block, KProcess::Stdin );

	QDir d;
	d.setPath( "/tmp" );
		if ( d.exists() )
		         d.remove( tmpFile );
}
