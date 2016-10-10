
#include <stdlib.h>
#include <iostream>
#include <qdir.h>
#include <qfile.h>
#include <qtstream.h>
#include <qstring.h>
#include <qlist.h>
#include <kapp.h>

class Match
{
public:
	Match( const char *f )
		{	weight = 1; filename = f; }

	void setMatch( int m )
		{	weight *= (m + 1); }

	const char *getFilename()
		{	return filename; }

	int getWeight()
		{	return weight; }

protected:
	QString filename;
	int weight;
};

class MatchList : public QList<Match>
{
protected:
	virtual int compareItems( GCI m1, GCI m2 )
	{
		if ( ((Match *)m1)->getWeight() > ((Match *)m2)->getWeight() )
			return -1;
		return 1;
	}
};

QString readTitle( const char *filename )
{
	QString title;

	QFile file( filename );

	if ( file.open( IO_ReadOnly ) )
	{
		QTextStream stream( &file );
		QString buffer;
		int pos;

		do
		{
			buffer = stream.readLine();
			if ( stream.eof() )
				return filename;
		}
		while ( ( pos = buffer.find( "<TITLE>", 0, FALSE ) ) < 0 );

		title = buffer.right( buffer.length() - pos - 7 );

		if ( ( pos = title.find( "</TITLE>", 0, FALSE ) ) > 0 )
			title.truncate( pos );
		else
		{
			do
			{
				buffer = stream.readLine();
				title += buffer;
				if ( stream.eof() )
					return title;
			}
			while ( ( pos = buffer.find( "</TITLE>", 0, FALSE ) ) < 0 );

			if ( ( pos = title.find( "</TITLE>", 0, FALSE ) ) > 0 )
				title.truncate( pos );
		}
	}

	return title;
}

int countOccurrences( const char *filename, const char *str )
{
	int count = 0;

	QString cmd = "grep -i -c ";	// -i ignore case, -c count occurrences
	cmd += str;
	cmd += ' ';
	cmd += filename;

	FILE *fp = popen( cmd, "r" );

	if ( fp )
	{
		char buffer[80];

		fgets( buffer, 80, fp );

		count = atoi( buffer );

		while ( !feof( fp ) ) fgetc( fp );

		pclose( fp );
	}

	return count;
}

int processFiles( MatchList &list, const char *dirname, const char **query )
{
	QDir files( dirname, "*.html", 0, QDir::Files | QDir::Readable );

	if ( !files.exists() )
		return 0;
	
	const QStrList *fileList = files.entryList();

	QStrListIterator itFile( *fileList );

	for ( ; itFile.current(); ++itFile )
	{
		QString filename = dirname;
		filename += "/";
		filename += itFile.current();

		Match *match = new Match( filename );

		for ( int i = 0; query[i]; i++ )
			match->setMatch( countOccurrences( filename, query[i] ) );

		if ( match->getWeight() > 1 )
			list.inSort( match );
		else
			delete match;
	}

	return 1;
}

int processDir( MatchList &list, const char *dirname, const char **query )
{
	QDir dir( dirname, "*", 0, QDir::Dirs );

	if ( !dir.exists() )
		return 0;
	
	const QStrList *dirList = dir.entryList();

	QStrListIterator itDir( *dirList );

	for ( ; itDir.current(); ++itDir )
	{
		if ( itDir.current()[0] == '.' )
			continue;

		QString filename = dirname;
		filename += "/";
		filename += itDir.current();

		processFiles( list, filename, query );

		processDir( list, filename, query );
	}

	return 1;
}

bool searchHTML( const char *search, ostream &stream )
{
	int i = 0;
	const char *query[20];
	char *buffer = new char [ strlen( search ) + 1 ];

	strcpy( buffer, search );
	query[i] = strtok( buffer, " " );

	while ( query[i] && i < 19 )
	{
		i++;
		query[i] = strtok( NULL, " " );
	}

	if ( i )
	{
		MatchList list;

		list.setAutoDelete( TRUE );

		QString dir = KApplication::kde_htmldir();
		processDir( list, dir, query );

		stream << "<H2>KDE Applications</h2>" << endl;

		if ( list.count() > 0 )
		{
			Match *match;

			for ( match = list.first(); match; match = list.next() )
			{
				stream << "<a href=\"file:" << match->getFilename() << "\">"
					<< readTitle( match->getFilename() ) << "</a><br>" << endl;
			}
		}
		else
		{
			stream << "No Matches" << endl;
		}

	}

	delete [] buffer;

	return 0;
}

