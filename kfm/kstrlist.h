#ifndef kstrlist_h
#define kstrlist_h

#include <qstrlist.h>

class KStrList : public QStrList
{
public:
    void copy( QStrList& _l )
    {
	clear();
	char *s;
	for ( s = _l.first(); s != 0L; s = _l.next() )
	    append( s );
    }
};

#endif
