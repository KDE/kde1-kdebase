// miscellaneous functions
//
// Copyright (C) Martin R. Jones 1995, 1996
//

#include <iostream.h>
#include <stdlib.h>
#include <ctype.h>
#include "misc.h"

#include "dbnew.h"

// duplicate a string - mem allocated with 'new'
char *StrDup(const char *src)
{
	if (!src) return NULL;

	char *dest = new char [strlen(src)+1];
	strcpy(dest, src);

	return dest;
}

char *StrUpperDup(const char *src)
{
	char *dest = new char [strlen(src)+1];
	unsigned i = 0;

	while (i < strlen(src))
	{
		dest[i] = toupper(src[i]);
		i++;
	}

	dest[i] = '\0';

	return dest;
}

const char *StrUpperStr(const char *haystack, const char *needle)
{
	if ((haystack == NULL) || (needle == NULL)) return NULL;

	char *uprHaystack = StrUpperDup(haystack);
	const char *ptr, *retPtr;

	ptr = strstr(uprHaystack, needle);

	if (ptr)
		retPtr = haystack + (ptr-uprHaystack);
	else
		retPtr = NULL;

	delete [] uprHaystack;

	return retPtr;
}

// Is this a safe command to issue via system, i.e. has a naughty user inserted
// special shell characters in a URL?
//
bool safeCommand( const char *cmd )
{
    if ( strpbrk( cmd, "&;`'\\\"|*?~<>^()[]{}$\n\r" ) )
        return false;

    return true;
}

