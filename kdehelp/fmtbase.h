//-----------------------------------------------------------------------------
// base for different help file format readers
//

#ifndef __FMT_BASE__
#define __FMT_BASE__

class cHelpFormatBase
{
public:
	virtual void Reset() = 0;
	virtual int  ReadLocation(const char *nodeName) = 0;
	virtual const char *GetTitle()	{	return NULL; }
	virtual const char *GetLocation()	{	return NULL; }

	virtual const char *PrevNode()	{	return NULL; }
	virtual const char *NextNode()	{	return NULL; }
	virtual const char *UpNode()	{	return NULL; }
	virtual const char *TopNode()	{	return NULL; }
	virtual const char *Dir()	{	return NULL; }
	virtual int  IsTop()	{	return 0; }
	virtual int  IsDir()	{	return 0; }
};

#endif
