#include "protocol.h"
#include "kio_errors.h"

#include <errno.h>
#include <stdio.h>

class KProtocolFILE :public KProtocol
{
private:
	long size;
	FILE *file;

public:
	KProtocolFILE();
	~KProtocolFILE();

	int Open(KURL *url, int mode);
	int Close();
	int Read(void *buffer, int len);
	int Write(void *buffer, int len);

	long Size();

	int atEOF();

	int MkDir(KURL *url);

        /**
	 * @return -1 if we dont know about permissions
	 */
        virtual int GetPermissions( KURL &_u );
        /**
	 * If '_perm' is -1, then we wont set any permissions here.
	 */
        virtual void SetPermissions( KURL &_u, int _perm );
};
