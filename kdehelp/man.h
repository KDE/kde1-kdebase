// man page parser class
//
// let 'man' do all the hard work - just interpret man's output.
//
// Copyright (C) Martin R. Jones 1995
//

// ============================================================================
// preprocessor directives

#ifndef __MAN_H__
#define __MAN_H__

#include <stdlib.h>
#include <fstream.h>
#include <qstring.h>
#include "fmtbase.h"
#include "misc.h"

#ifndef MAN_SUCCESS
#define MAN_SUCCESS			1		// the return value from successful man
#endif

#define MAN_SECTIONS		"1:2:3:4:5:6:7:8:9:n"
#define MAN_ENV			    "MANPATH"
#define MAN_MAXSECTIONS		20
#define MAN_MAXPATHS		50

#define MAN_POSSECTION		-1
#define MAN_POSDIR		    -2

#define MAN_DIRECTORY		"index"


// ============================================================================

class cPage
{
public:
	cPage(const char *theName)
        { _name = StrDup(theName); _next = NULL; }
	cPage()	{	_name = NULL; _next = NULL; }
	~cPage()	{	if (_name) delete [] _name; }

    cPage *next() const { return _next; }
    void setNext( cPage *n ) { _next = n; }

    const char *name() const { return _name; }

private:
	char *_name;
	cPage *_next;
};

// ----------------------------------------------------------------------------
class cManSection
{
private:
	cPage *head, *tail, *curr;
	char	*name;
	char	desc[80];
	int		numPages;
	char	isRead;

	static char	*searchPath[MAN_MAXPATHS];
	static int	numPaths;
    static int  sectCount;

public:
	cManSection(const char *theName);
	~cManSection();

	int  NumPages()			{	return numPages; }
	char *GetName()			{	return name; }
	char *GetDesc()			{	return desc; }

	cPage *MoveToHead() {	return curr = head; }
	cPage *Next()	{	return curr = curr->next(); }
	cPage *Get()	{	return curr; }
	void ReadSection();
	void ReadDir(const char *dirName);
	void AddPage(const char *pageName);
	cPage *FindPage( const char *pageName );
};

// ============================================================================
class cMan : public cHelpFormatBase
{
public:
	QString			HTMLPage;
	int			pos;
	char			posString[80];
	char			staticBuffer[80];

public:
	cMan();
	virtual ~cMan();

	virtual void Reset() {}
	virtual int  ReadLocation(const char *name);
	virtual const char *GetTitle()	{	return posString; }
	virtual const char *GetLocation()	{	return posString; }

	virtual const char *PrevNode();
	virtual const char *NextNode()	{	return NULL; }
	virtual const char *UpNode()	{	return TopNode(); }
	virtual const char *TopNode();
	virtual const char *Dir()		{	return MAN_DIRECTORY; }

	virtual int  IsTop()
		{	return (pos != MAN_POSDIR) && (pos != MAN_POSSECTION); }
	virtual int  IsDir()
		{	return (pos != MAN_POSDIR); }

	const char *page() { return HTMLPage; }

private:
	static int instance;
};

#endif

