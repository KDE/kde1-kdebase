//----------------------------------------------------------------------------
//
// Maintain a history of sites visited
//
// Copyright (c) 1997 The KDE project.
//

#include <qdatetm.h>
#include <qdict.h>

class KHistory
{
public:
    KHistory( const char *_file );
    ~KHistory();

    void setLifetime( int lt )
	{  lifeTime = lt; }

    void addURL( const char *_url );
    inline bool inHistory( const char *_url );

    void loadHistory();
    void saveHistory();

private:
    QDict<QDate> dict;
    QString filename;
    int lifeTime;
};

inline bool KHistory::inHistory( const char *_url )
{
    return ( dict.find( _url ) != 0 ? true : false );
}

