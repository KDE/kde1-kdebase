#ifndef __DBNEW_H__
#define __DBNEW_H__

#ifdef DEBUGNEW

#define NEWARRAY

#include <stdlib.h>


//=============================================================================
// Function Prototypes

void *operator new(size_t Size);
void *operator new(size_t Size, const char *Filename, int Line);
#ifdef NEWARRAY
void *operator new[](size_t Size, const char *Filename, int Line);
void operator delete[](void *Mem);
#endif
void operator delete(void *Mem);

void SetDBFileLinePos(const char *TheFilename, int TheLine);

//=============================================================================
// preprocessor directives

#define new new(__FILE__, __LINE__)
#define delete SetDBFileLinePos(__FILE__, __LINE__), delete

#define DBNewVerbose	DBNew_Verbose = 1;
#define DBNewQuiet		DBNew_Verbose = 0;

#define DBN_MAGIC1		'H'
#define DBN_MAGIC2		'i'

#define DBN_ARRAY		1

//=============================================================================

extern int DBNew_Verbose;



#else		// DEBUGNEW

//=============================================================================
// preprocessor directives

#define DBNewVerbose
#define DBNewQuiet

#endif		// DEBUG

#endif		// __DBNEW_H__

