#ifndef _subprotocol_h
#define _subprotocol_h

#ifndef _KSUBPROTCOL_INCLUDED_
#define _KSUBPROTCOL_INCLUDED_

#include "protocol.h"

class KSubProtocol : public KProtocol
{
    Q_OBJECT
protected:
	KURL ParentURL;
	QString ParentName;
	void InitParent();
	KProtocol *Parent;

public:
	void InitSubProtocol(char *_ParentName, KURL _ParentURL);
	KSubProtocol();
};
#endif

#endif
