#ifndef _protocol_h
#define _protocol_h

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

#include "kioslave_ipc.h"

class KProtocolDirEntry
{
public:
	QString name;
	QString date;
	QString access;
	QString owner;
	QString group;

	long size;
	bool isdir;
};

class KProtocol : public QObject
{
    Q_OBJECT
signals:
       /**
	* Emitted if the protocol can tell us something about the mime-type.
	* For example HTTP can. The signal is only connected to the IPC
	* mechanism during a call to @ref KIOSlave::get.
	*/
       void mimeType( const char* );
       /**
	* Called if the given URL is redirected. For example HTTP does this by
	* sending a "Location:" line in its header. The signal is only connected to the IPC
	* mechanism during a call to @ref KIOSlave::get.
	*/
       void redirection( const char* );

    void info( const char* );
    
    void cookie( const char*, const char*);
    
private:
	QString lastErrorMessage;
	int lastKError;
	int lastSysError;

protected:
	int Error(int KError, QString ErrorMessage, int SysError = 0);
	int NotImplemented();

public:
	enum Kind { READ = 1, WRITE = 2, OVERWRITE = 4 }; 

	enum Status { FAIL = -1, SUCCESS = 0};

	/** functions for files/objects **/
	virtual int Open(KURL *url, int mode);
	
	/** Opening for reload (no cache) **/
	virtual int ReOpen(KURL *url, int mode);

	/** Set data for POST method **/
    virtual int SetData(const char *) { return FAIL; }
	/** Set cookies **/
    virtual int SetCookies(const char *) { return FAIL; }
  	virtual long Read(void *buffer, long nbytes);
	virtual long Write(void *buffer, long nbytes);
	virtual int Close();
	virtual int atEOF();
	virtual long Size();

	virtual int Delete(KURL *url);

	/** functions for directories **/
	virtual int OpenDir(KURL *url);
        /**
	 * @return TRUE if one made a call to @ref #OpenDir and if the result of
	 *              this is some HTML. For example the file protocol will
	 *              devliver HTML code if it finds some "index.html" file in
	 *              the directory.
	 */
        virtual bool isHTML() { return FALSE; }
        /**
	 * Allows or disallows @ref #OpenDir to deliver HTML code instead of
	 * @ref #KProtocolDirEntry items.
	 */
        virtual void AllowHTML( bool ) { }
        /**
	 * Call this function if a call top @ref #OpenDir results in some HTML code.
	 * This function will use '_ipc' to emit this HTML code.
	 * This is the alternative to @ref #ReadDir depending on @ref #isHTML.
	 */
        virtual void EmitData( KIOSlaveIPC* ) { };
	virtual KProtocolDirEntry *ReadDir();
	virtual int CloseDir();
	virtual int MkDir(KURL *url);

	/** errorhandling **/
	void GetLastError(int& KError, QString& ErrorMessage, int &SysError);

        /**
         * @return -1 if we dont know about permissions
         */
        virtual int GetPermissions( KURL &_u );
        /**
         * If '_perm' is -1, then we wont set any permissions here.
         */
        virtual void SetPermissions( KURL &_u, int _perm );   
};

#define _KPROTCOL_INCLUDED_
#endif

#endif
