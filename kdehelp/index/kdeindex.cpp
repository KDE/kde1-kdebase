//-----------------------------------------------------------------------------
// help index builder for kdehelp
//
// (c) Martin R. Jones 1997

#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <qlist.h>
#include <qdir.h>
#include <kapp.h>
#include <ksimpleconfig.h>
#include <klocale.h>

//-----------------------------------------------------------------------------

class Entry
{
public:
	int readEntry( const char *filename );
	void writeHTML( QTextStream &output );

protected:
	QString docPath;
	QString info;
	QString name;
};
	

int Entry::readEntry( const char *filename )
{
	QFile file( filename );
	if ( !file.open( IO_ReadOnly ) )
		return FALSE;

	file.close(); // kalle

	KSimpleConfig config( filename, true );
	config.setGroup( "KDE Desktop Entry" );

	QString path = config.readEntry( "DocPath" );
	if ( path.isNull() )
		return FALSE;
	docPath = kapp->kde_htmldir() + "/default/";
	docPath += path;
	info = config.readEntry( "Info" );
	if ( info.isNull() )
		info = config.readEntry( "Comment" );

	name = config.readEntry( "Name" );

	if ( name.isNull() )
	{
		const char *p = strrchr( filename, '/' );
		if ( p )
			name = p + 1;
		else
			name = filename;
		int pos;
		if ( ( pos = name.findRev( ".kdelnk" ) ) > 0 )
		{
			name = name.left( pos );
		}
	}

	return TRUE;
}

void Entry::writeHTML( QTextStream &stream )
{
	stream << "<dt><a href=file:\"" << docPath << "\"><b>" << name
		 << "</b></a>" << endl;
	stream << "<dd>" << info << endl;
}

//-----------------------------------------------------------------------------

int readEntries( const char *dirName, QList<Entry> &list )
{
	QDir fileDir( dirName, "*.kdelnk", 0, QDir::Files | QDir::Hidden | QDir::Readable );

	if ( !fileDir.exists() )
		return 0;

	const QStrList *fileList = fileDir.entryList();

	QStrListIterator itFile( *fileList );

	for ( ; itFile.current(); ++itFile )
	{
		Entry *entry = new Entry;
		QString filename = dirName;
		filename += "/";
		filename += itFile.current();

		if ( entry->readEntry( filename ) )
			list.append( entry );
		else
			delete entry;
	}

	return list.count();
}

int processDir( const char *dirName, QTextStream &stream )
{
	QList<Entry> list;

	list.setAutoDelete( TRUE );

	QDir dirDir( dirName, "*", 0, QDir::Dirs );

	if ( !dirDir.exists() )
		return 0;

	const QStrList *dirList = dirDir.entryList();

	QStrListIterator itDir( *dirList );

	for ( ; itDir.current(); ++itDir )
	{
		if ( itDir.current()[0] == '.' )
			continue;

		QString filename = dirName;
		filename += "/";
		filename += itDir.current();

		stream << "<blockquote>";

		QString dirFile = filename + "/.directory";
		if ( QFile::exists( dirFile ) )
		{
		    KSimpleConfig sc( dirFile, true );
		    sc.setGroup( "KDE Desktop Entry" );
		    QString dirName = sc.readEntry( "Name", itDir.current() );
		    stream << "<h2>" << dirName << "</h2>" << endl;
		}
		else
		    stream << "<h2>" << itDir.current() << "</h2>" << endl;


		if ( readEntries( filename, list ) > 0 )
		{
			stream << "<dl>";
			for ( Entry *entry = list.first(); entry; entry = list.next() )
				entry->writeHTML( stream );
			stream << "</dl>";
		}

		list.clear();

		processDir( filename, stream );
		stream << "</blockquote>" << endl;
	}

	return TRUE;
}

int main( int argc, char **argv )
{
	KApplication a( argc, argv, "helpindex" );

	QString home = getenv( "HOME" );

	QTextStream stream( stdout, IO_WriteOnly );

	stream << "Content-type: text/html" << endl << endl;
	stream << "<html><head><title>" << klocale->translate( "KDE Applications Index" ) <<
		"</title></head>" << endl;
	stream << "<body>" << endl;
	stream << "<h1>" << klocale->translate( "KDE Applications Index" ) << "</h1>" << endl;

	// System applications
	QString appPath = kapp->kde_appsdir();

	QList<Entry> list;
	list.setAutoDelete( TRUE );

	// first read entries in this directories
	if ( readEntries( appPath, list ) > 0 )
	{
		stream << "<dl>" << endl;
		for ( Entry *entry = list.first(); entry; entry = list.next() )
			entry->writeHTML( stream );
		stream << "</dl>" << endl;
	}
	list.clear();

	// then process all subdirectories
	processDir( appPath, stream );

	// User applications
	appPath = KApplication::localkdedir() + "/share/applnk";

	// first read entries in this directories
	if ( readEntries( appPath, list ) > 0 )
	{
		stream << "<dl>" << endl;
		for ( Entry *entry = list.first(); entry; entry = list.next() )
			entry->writeHTML( stream );
		stream << "</dl>" << endl;
	}
	list.clear();

	// then process all subdirectories
	processDir( appPath, stream );

	stream << "</body></html>" << endl;

	return 0;
}

