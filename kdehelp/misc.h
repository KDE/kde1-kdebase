// miscellaneous functions
//
// Copyright (C) Martin R. Jones 1995
//

#ifndef __MISC_H__
#define __MISC_H__

#include <string.h>
#include <qlist.h>

char *StrDup(const char *src);
char *StrUpperDup(const char *src);
const char *StrUpperStr(const char *haystack, const char *needle);
bool safeCommand( const char *cmd );


template<class T> class cHistory
{
public:
	cHistory(unsigned _maxLen)
	{
		maxLen = _maxLen;
		list.setAutoDelete(TRUE);
	}

	T *Back()	{	return list.prev(); }
	T *Forward()	{	return list.next(); }
	T *Current()	{	return list.current(); }

	int IsBack()	{	return (list.current() != list.getFirst()); }
	int IsForward()	{	return (list.current() != list.getLast()); }

	void Add( T *item )
	{
		T *stop = list.current();
		if (stop)
			while (list.last() != stop) list.removeLast();
		if (list.count() > maxLen) list.removeFirst();
		list.append( item );
	}

    const cHistory<T> &operator=( const cHistory<T> &hist )
    {
        QListIterator<T> it( hist.list );
        list.clear();
        for ( ; it.current(); ++it )
        {
            list.append( new T( *it.current() ) );
        }
        if ( hist.list.at() >= 0 )
            list.at( hist.list.at() );
        return *this;
    }

private:
	unsigned maxLen;
	QList<T> list;
};

#endif

