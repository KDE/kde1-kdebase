// Error management class
//
// contains error status and last error
//
// Copyright (C) Martin R. Jones 1995
//

#include <stdlib.h>
#include <iostream.h>
#include "error.h"

cError Error;

// ============================================================================
// The default error handler
//
void DefHandler(int type, const char *theMsg)
{
	if (type == ERR_FATAL)
	{
		cout << "Fatal Error: " << theMsg << endl;
		exit(1);
	}
	else
	{
		cout << "Warning: " << theMsg << endl;
	}
}

// ============================================================================
//
cError::cError()
{
	SetHandler(DefHandler);
	type = ERR_NONE;
}

void cError::Set(int theType, const char *theDesc)
{
	type = theType;
	desc = theDesc;

	handler(type, desc);

	if (type == ERR_WARNING) type = ERR_NONE;
}

