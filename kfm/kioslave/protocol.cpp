#include "protocol.h"
#include "kio_errors.h"

int KProtocol::Error(int KError, QString ErrorMessage, int SysError)
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

int KProtocol::Open(KURL *, int)			{ return(NotImplemented()); }
int KProtocol::ReOpen(KURL *_url, int _reload)			{ return(Open(_url,_reload)); }
int KProtocol::Close()						{ return(NotImplemented()); }
long KProtocol::Read(void *, long)			{ return(NotImplemented()); }
long KProtocol::Write(void *, long)			{ return(NotImplemented()); }
int KProtocol::atEOF()						{ return(NotImplemented()); }
long KProtocol::Size()						{ return(NotImplemented()); }

int KProtocol::OpenDir(KURL *)				{ return(NotImplemented()); }
int KProtocol::CloseDir()					{ return(NotImplemented()); }
int KProtocol::MkDir(KURL *)				{ return(NotImplemented()); }
KProtocolDirEntry *KProtocol::ReadDir()		{ return(NULL); }

int KProtocol::GetPermissions( KURL & ) { return -1; }
void KProtocol::SetPermissions( KURL &, int ) { }

#include "protocol.moc"
