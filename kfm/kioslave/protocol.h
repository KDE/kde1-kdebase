/*----------------------------------------------------

still missing:
==============
- link method
- delete method
- rename method
- configure method/widget
- complain if url is invalid (no such host, http:// is
  not a valid url, file:/ is perfectly legal ...)

-------------------------------------------------------*/

#ifndef _KPROTCOL_INCLUDED_
#include <kurl.h>

class KProtocolDirEntry
{
public:
	QString name;
	QString date;
	QString access;
	QString owner;
	QString group;

	int size;
	bool isdir;
};

class KProtocol
{
private:
	QString lastErrorMessage;
	int lastKError;
	int lastSysError;

protected:
	int Error(int KError, QString ErrorMessage, int SysError = 0);
	int NotImplemented();

public:
	const int READ=1, WRITE=2, OVERWRITE=4; 

	const int FAIL = -1, SUCCESS = 0;

	/** functions for files/objects **/
	virtual int Open(KURL *url, int mode);
	virtual int Read(void *buffer, int nbytes);
	virtual int Write(void *buffer, int nbytes);
	virtual int Close();
	virtual int atEOF();
	virtual long Size();

        /**
	 * @return -1 if we dont know about permissions
	 */
        virtual int GetPermissions( KURL &_u );
        /**
	 * If '_perm' is -1, then we wont set any permissions here.
	 */
        virtual void SetPermissions( KURL &_u, int _perm );
    
	/** functions for directories **/
	virtual int OpenDir(KURL *url);
	virtual KProtocolDirEntry *ReadDir();
	virtual int CloseDir();
	virtual int MkDir(KURL *url);

	/** errorhandling **/
	void GetLastError(int& KError, QString& ErrorMessage, int &SysError);
};

#define _KPROTCOL_INCLUDED_
#endif
