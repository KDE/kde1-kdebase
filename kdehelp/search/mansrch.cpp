
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dirent.h>
#include "mansrch.h"

const char *FindXRef( const char *theText );

#define MAXSECTIONLEN  4
#define NUMSECTIONS    10

// ----------------------------------------------------------------------------
// use apropos to search man
//
bool searchMan( const char *search, ostream &stream )
{
	char syscmd[256];
	char buffer[1024];
	char tmp[256];
	const char *pos, *ptr;

	FILE *fp;

	stream << "<H2>Online Manuals</H2>" << endl;

	sprintf( syscmd, "apropos %s", search );

	fp = popen( syscmd, "r" );

	if ( !fp )
		return false;

	while ( !feof( fp ) )
	{
		if ( fgets( buffer, 1024, fp ) == NULL )
			break;

		if ( strchr( buffer, 27 ) )	// escape sequence
			continue;

		pos = buffer;
		ptr = FindXRef( pos );	// check for cross reference in this line

		while ( ptr )
		{
			strncpy( tmp, pos, ptr-pos );
			tmp[ptr-pos] = '\0';
			stream << tmp;

			pos = strchr( ptr, ')' ) + 1;
			strncpy( tmp, ptr, pos-ptr );
			tmp[pos-ptr] = '\0';
			stream << "<a href=\"man:" << tmp << "\">" << tmp << "</a>" << endl;

			ptr = FindXRef(pos);
		}

		stream << pos << "<br>" << endl;
	}

	pclose( fp );

	return true;
}

const char *FindXRef( const char *theText )
{
	static char *sections[] = { "1", "2", "3", "4", "5", "6",
								 "7", "8", "9", "n" };
	const char *ptr, *ptr1, *xrefPtr;
	int i;

	ptr = strchr( theText, '(' );

	if ( ptr )
	{
		ptr1 = strchr( ptr, ')' );
		if ( ptr1 )
		{
			if ( ( ptr1-ptr-1 > MAXSECTIONLEN ) || ( ptr1-ptr <= 1 ) )
				return NULL;

			for ( i = 0; i < NUMSECTIONS; i++ )
			{
				if ( !strncmp( ptr+1, sections[i], strlen( sections[i] ) ) )
				{
					xrefPtr = ptr-1;

					// this allows 1 space between name and '('
					if ( *xrefPtr == ' ' ) xrefPtr--;
					if ( *xrefPtr == ' ' ) return NULL;

					while ( ( xrefPtr > theText ) && ( *(xrefPtr-1) != ' ' ) )
						xrefPtr--;

					return xrefPtr;
				}
			}
		}
		else
			return NULL;
	}

	return NULL;
}

