#include "subprotocol.h"
#include "manage.h"

KSubProtocol::KSubProtocol()
{
	Parent = NULL;
}

void KSubProtocol::InitParent()
{
	if(!Parent) Parent = CreateProtocol(ParentName.data());
}

void KSubProtocol::InitSubProtocol(char *_ParentName, KURL _ParentURL)
{
	ParentURL = _ParentURL;
	ParentName = _ParentName;
}

#include "subprotocol.moc"
