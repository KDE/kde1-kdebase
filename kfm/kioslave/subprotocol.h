#ifndef _subprotocol_h
#define _subprotocol_h

#include "protocol.h"

class KSubProtocol : public KProtocol
{
    Q_OBJECT
protected:
	QString ParentURL;
	void InitParent();
	KProtocol *Parent;

public:
	void InitSubProtocol( const char *_ParentURL );
	KSubProtocol();
};

#endif
