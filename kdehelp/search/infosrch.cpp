
#include <qstring.h>
#include <qfile.h>
#include <qtstream.h>

int searchInfoFile( const char *filename, const char *str )
{
	QFile file( filename );

	if ( file.open( IO_ReadOnly ) )
	{
	}
}

