#ifndef _tar_h
#define _tar_h

#include "subprotocol.h"
#include "slave.h"

class KProtocolTAR :public KSubProtocol
{
    Q_OBJECT
private:
	KSlave Slave;
	char *dirpathmem, *dirpath;
	FILE *dirfile;
	int _EOF;

public:
/*
	KProtocolTAR();
	~KProtocolTAR();
*/

	int HandleRefill();

	int OpenDir(KURL *url);
	KProtocolDirEntry *ReadDir();
	int CloseDir();

	int AttachTAR(char *commandstr);
	int Open(KURL *url, int mode);
	int Close();
	long Read(void *buffer, long len);

	long Size();

        int atEOF();
};

#endif
