#include "subprotocol.h"
#include "manage.h"

KSubProtocol::KSubProtocol()
{
	Parent = NULL;
}

void KSubProtocol::InitParent()
{
	if(!Parent) Parent = CreateProtocol( ParentURL );
}

void KSubProtocol::InitSubProtocol( const char *_ParentURL )
{
	ParentURL = _ParentURL;
}

#include "subprotocol.moc"
