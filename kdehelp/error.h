// Error management class
//
// contains error status and last error (at least I will one day)
//
// Copyright (C) Martin R. Jones 1995
//

#ifndef __ERROR_H__
#define __ERROR_H__

#define ERR_NONE		0
#define ERR_FATAL		1
#define ERR_WARNING		2

void DefHandler(int type, const char *theMsg);

// ============================================================================
class cError
{
	int type;
	const char *desc;
	void (*handler)(int, const char *);

public:
	cError();

	void Set(int theType, const char *theDesc);

	void SetHandler(void (*theHandler)(int, const char *))
		{	handler = theHandler; }
};

extern cError Error;

#endif

