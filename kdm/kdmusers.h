// -*-C++-*-

# include "kdm-config.h"

#include "kdmview.h"

class KDMUserItem : public KDMViewItem {
public:
     setName( const char* name) { user_name = QString( name);}
     const char* name() { return user_name.data()} cont;
private:
     QString user_name;
};
