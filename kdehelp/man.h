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
#include "fmtbase.h"
#include "misc.h"

#ifndef MAN_SUCCESS
#define MAN_SUCCESS			1		// the return value from successful man
#endif

#define MAN_SECTIONS		"1:2:3:4:5:6:7:8:9:n"
#define MAN_ENV				"MANPATH"
#define MAN_MAXSECTIONS		20
#define MAN_MAXPATHS		20

#define MAN_TEXT			1
#define MAN_UNDERLINE		2
#define MAN_BOLD			3
#define MAN_HEADING1		4
#define MAN_HEADING2		5
#define MAN_HEADING3		6
#define MAN_CR				7
#define MAN_XREF			8
#define MAN_DIR				9

#define MAN_DIRPAGE			1
#define MAN_DIRBOOK			2

#define MAN_POSSECTION		-1
#define MAN_POSDIR			-2

#define MAN_DIRECTORY		"index"


// ============================================================================

class cPage
{
public:
	char *name;
	cPage *next;

public:
	cPage(const char *theName) {	name = StrDup(theName); next = NULL; }
	cPage()	{	name = NULL; next = NULL; }
	~cPage()	{	if (name) delete [] name; }
};

// ----------------------------------------------------------------------------
class cManSection
{
private:
	cPage *head, *tail, *curr;
	char	*searchPath[MAN_MAXPATHS];
	char	*name;
	char	desc[80];
	int		numPaths;
	int		numPages;
	char	isRead;

public:
	cManSection(const char *theName);
	~cManSection();

	int  NumPages()			{	return numPages; }
	char *GetName()			{	return name; }
	char *GetDesc()			{	return desc; }

	cPage *MoveToHead() {	return curr = head; }
	cPage *Next()	{	return curr = curr->next; }
	cPage *Get()	{	return curr; }
	void ReadSection();
	void ReadDir(const char *dirName);
	void AddPage(const char *pageName);
	cPage *FindPage( const char *pageName );
};

// ============================================================================

class cManBase
{
public:
	char type;
	char *text;
	cManBase *next;
	cManBase *prev;

public:
	cManBase(char theType)
		{   type = theType; text = NULL; prev = next = NULL; }
	virtual ~cManBase() {	if (text) delete [] text; }
};

// ----------------------------------------------------------------------------
class cManText : public cManBase
{
public:
	cManText(char theType = MAN_TEXT) : cManBase(theType) {}
};

// ----------------------------------------------------------------------------
class cManXRef : public cManBase
{
public:
	char *page;

public:
	cManXRef(char theType = MAN_XREF) : cManBase(theType) {	page = NULL; }
	virtual ~cManXRef()	{	if (page) delete [] page; }
};


// ----------------------------------------------------------------------------
class cManDir : public cManBase
{
public:
	char dirType;
	char *page;
	char *desc;

public:
	cManDir(char theType = MAN_DIR) : cManBase(theType) { page = desc = NULL; }
	virtual ~cManDir()	{	if (page) delete [] page; if (desc) delete [] desc; }
};

// ----------------------------------------------------------------------------
class cManTextList
{
private:
	cManBase *head;
	cManBase *tail;
	cManBase *curr;

public:
	cManTextList();
	~cManTextList();

	void Reset();
	int  MakeDirectory();
	int  Read(cManSection &sect);
	int  Read(ifstream &stream);

	void ExtractText(char *in, char *out, int len);
	void RemoveFooter();
	void Add(cManBase *manBase);
	void AddCR();
	void AddText(const char *theText, char theMode);
	void AddXRef(const char *theText, const char *theRef);
	void AddDir(const char *theText, const char *theRef, const char *theDesc,
				char theType);
	const char *FindXRef(const char *theText);

	cManBase *GotoHead()	{	return curr = head; }
	cManBase *Next()		{	return curr = curr->next; }
	cManBase *Get()			{	return curr; }

	void Print()			{}
};


// ============================================================================
class cMan : public cHelpFormatBase
{
public:
	cManTextList	manList;
	int				pos;
	char			posString[80];
	char			staticBuffer[80];

public:
	cMan();
	virtual ~cMan();

	virtual void Reset();
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

private:
	static int instance;
};

#endif

