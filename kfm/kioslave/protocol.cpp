#include "protocol.h"
#include "kio_errors.h"
#include <config-kfm.h>

int KProtocol::Error(int KError, QString ErrorMessage, int SysError = 0)
{
	lastErrorMessage = ErrorMessage;
	lastKError = KError;
	lastSysError = SysError;
	return(FAIL);	// more convenient, because you can use return(Error(...));
}

void KProtocol::GetLastError(int& KError, QString& ErrorMessage, int &SysError)
{
	SysError = lastSysError;
	KError = lastKError;
	ErrorMessage = lastErrorMessage;
}

int KProtocol::NotImplemented()
{
	return(Error(KIO_ERROR_NotImplemented, "This is not implemented..."));
}

int KProtocol::Open(KURL *url, int mode)	{ return(NotImplemented()); }
int KProtocol::Close()						{ return(NotImplemented()); }
int KProtocol::Read(void *buffer, int len)	{ return(NotImplemented()); }
int KProtocol::Write(void *buffer, int len)	{ return(NotImplemented()); }
int KProtocol::atEOF()						{ return(NotImplemented()); }
long KProtocol::Size()						{ return(NotImplemented()); }

int KProtocol::GetPermissions( KURL &_u ) { debugT("Permissions not impl.\n"); return -1; }
void KProtocol::SetPermissions( KURL &_u, int _perm ) { debugT("Permissions not impl.\n"); }

int KProtocol::OpenDir(KURL *url)			{ return(NotImplemented()); }
int KProtocol::CloseDir()					{ return(NotImplemented()); }
int KProtocol::MkDir(KURL *url)				{ return(NotImplemented()); }
KProtocolDirEntry *KProtocol::ReadDir()		{ return(NULL); }
