#ifndef _kintlist_h
#define _kintlist_h

#include <qlist.h>

typedef QList<int> BaseIntList; 

class KIntList : public BaseIntList
{
  public:
    KIntList() : BaseIntList(){ }
    virtual ~KIntList() { }

  protected:
    virtual int compareItems( GCI item1, GCI item2);
};

#endif //_kintlist_h
