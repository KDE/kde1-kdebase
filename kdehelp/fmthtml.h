//-----------------------------------------------------------------------------
// Simple class to keep HTML docs in line with other help formats
//

#ifndef __FMT_HTML__
#define __FMT_HTML__

#include <unistd.h>
#include "fmtbase.h"


class cHTMLFormat : public cHelpFormatBase
{
private:
	QString location;

public:
	virtual ~cHTMLFormat() {}
	virtual void Reset()	{}
	virtual int  ReadLocation( const char *_location )
			{	location = _location; return access( _location, R_OK ); }
	virtual const char *GetLocation()	{	return location; }

	virtual const char *PrevNode()	{	return NULL; }
	virtual const char *NextNode()	{	return NULL; }
	virtual const char *UpNode()	{	return NULL; }
	virtual const char *TopNode()	{	return NULL; }
	virtual const char *Dir()	{	return "(index)"; }
	virtual int  IsTop()	{	return 0; }
	virtual int  IsDir()	{	return 1; }
};

#endif

