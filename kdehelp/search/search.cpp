//----------------------------------------------------------------------------
// Very simple documentation search
//

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <qstring.h>
#include <kapp.h>
#include "mansrch.h"
#include "htmlsrch.h"

const char *parseQuery( const char *str, QString &name, QString &value );
QString decodeQuery( QString &str );

int main()
{
	const char *qs = getenv( "QUERY_STRING" );

	if ( qs == NULL )
		return 1;

	const char *str = qs;
	QString name, value;

	QString search = "";
	bool man = false, HTML = false, info = false;

	do
	{
		str = parseQuery( str, name, value );

		if ( strncasecmp( name, "Search", 7 ) == 0 )
		{
			search = value;
		}
		if ( strncasecmp( name, "Category", 8 ) == 0 )
		{
			if ( strncasecmp( value, "KDE", 3 ) == 0 )
				HTML = true;
			else if ( strncasecmp( value, "man", 3 ) == 0 )
				man = true;
			else if ( strncasecmp( value, "info", 3 ) == 0 )
				info = true;
		}
	}
	while ( str );

	cout << "Content-type: text/html" << endl << endl;
	cout << "<HTML><HEAD><TITLE>Search Results: " << search;
	cout << "</TITLE></HEAD><BODY>" << endl;
	cout << "<H1>Search Results: " << search << "</H1>" << endl;

	if ( HTML )
		searchHTML( search, cout );
	if ( man )
		searchMan( search, cout );

	cout << "</BODY></HTML>" << endl;

	return 0;
}

const char *parseQuery( const char *str, QString &name, QString &value )
{
	const char *end = strchr( str, '&' );

	name = value = "";

	QString tmp, field = str;

	if ( end )
	{
		field.truncate( end - str );
		end++;
	}

	int pos;

	if ( ( pos = field.find( '=' ) ) > 0 )
	{
		tmp = field.left( pos );
		name = decodeQuery( tmp );
		tmp = field.right( field.length() - pos - 1 );
		value = decodeQuery( tmp );
	}

	return end;
}

QString decodeQuery( QString &str )
{
	unsigned pos = 0;
	QString decoded = "";

	do
	{
		unsigned char c = str[pos];

		if ( c == '+' )
			decoded += ' ';
		else if ( c == '%' )
		{
			char num[3];
			num[0] = str[++pos];
			num[1] = str[++pos];
			num[2] = '\0';
			int val = strtol( num, NULL, 16 );
			decoded += (unsigned char)val;
		}
		else
			decoded += c;

		pos++;
	}
	while ( pos < str.length() );

	return decoded;
}

