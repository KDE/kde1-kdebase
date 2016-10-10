// Info file parser classes
//
// classes for reading texinfo info files
//
// Copyright (C) Martin R. Jones 1995
//

#ifndef __INFO_H__
#define __INFO_H__

#include <cstdlib>
#include <fstream>
#include "fmtbase.h"
#include "dbnew.h"

using namespace std;

#define INFO_ENV				"INFOPATH"
#define INFO_MAXPATHS			25
#define INFO_HISTORYLEN			50

#define INFO_MARKER				'\037'
#define INFO_TAGSEPARATOR		"\177"

#define INFO_FILE_TOKEN			"File"
#define INFO_NODE_TOKEN			"Node"
#define INFO_NEXT_TOKEN			"Next"
#define INFO_PREV_TOKEN			"Prev"
#define INFO_UP_TOKEN			"Up"

#define INFO_MENU_TOKEN			"* Menu:"
#define INFO_XREF_TOKEN			"*Note "
#define INFO_XREF_TOKEN1		"*note "
#define INFO_INDIRECT_TOKEN		"Indirect:"
#define INFO_TAGTABLE_TOKEN		"Tag Table:"
#define INFO_TAGTABLEEND_TOKEN	"End Tag Table"

#define INFO_NODETEXT			1
#define INFO_NODEINLINE			2
#define INFO_NODEMENU			3
#define INFO_NODEMENUCONT		4
#define INFO_NODEXREF			5
#define INFO_NODEHEADING		6

#define INFO_HEADING1			1
#define INFO_HEADING2			2
#define INFO_HEADING3			3

// ============================================================================
// Indirect info file structs and classes

// ----------------------------------------------------------------------------
// Indirect structure - hold indirect file attributes
// file 	: filename of indirect file
// pos		: where this fits in the overall scheme of things
class cIndirect
{
public:
    cIndirect( const char *f, int off );
    ~cIndirect();

    const char *file() const { return mFile; }
    int offset() const { return mOffset; }
    cIndirect *next() const { return mNext; }

    void setNext( cIndirect *n )
        { mNext = n; }

protected:
	int mOffset;
	char *mFile;
	cIndirect *mNext;
};

// ----------------------------------------------------------------------------
class cIndirectList
{
private:
	cIndirect *head;
	cIndirect *tail;

public:
	cIndirectList();
	~cIndirectList();

	void Reset();
	int  Read(const char *filename);
	void Add(const char *name, int offset);
	const char *Find(int &offset);
	void Print();
};

// ============================================================================
// Structures and classes used for tag table

// ----------------------------------------------------------------------------
// Tag structure - holds tag attributes
// node 	: node name
// offset	: offset into file (may be into indirect files)

class cTag
{
public:

	cTag( const char *n, int off);
	~cTag();

    const char *node() const { return mNode; }
    int offset() const { return mOffset; }
    cTag *next() const { return mNext; }

    void setNext( cTag *n ) { mNext = n; }

protected:
	char *mNode;
	int  mOffset;
	cTag *mNext;
};

// ----------------------------------------------------------------------------
// tag table class - look after tags, searching, etc.

class cTagTable
{
private:
	cTag *head;
	cTag *tail;
	cTag *curr;

public:
	cTagTable();
	~cTagTable();

	void Reset();
	void Read(const char *filename);
	void CreateTable(const char *filename);
	void Add(char *node, int offset);
	int  Find(char *node);
	void GotoHead()		{	curr = head; }
	cTag *Get()			{	return curr; }
	void Next()			{	curr = curr->next(); }
	void Print();
};


// ============================================================================
// node related classes

// ----------------------------------------------------------------------------

class cNodeLine
{
public:
	char type;
	cNodeLine *next;

public:
	cNodeLine(char theType) {	type = theType; next = NULL; }
	virtual ~cNodeLine() {}
};


// ----------------------------------------------------------------------------
class cNodeText : public cNodeLine
{
public:
	char *_text;

public:
	cNodeText(char theType = INFO_NODETEXT) : cNodeLine(theType) {	_text = NULL; }
	void setText( const char *t )
	{
		if (_text)
			delete [] _text;
		_text = NULL;
		if (!t)
			return;
		_text = new char [strlen(t)+1];
		strcpy(_text, t);
	}
	const char *text() { return _text; }
	virtual ~cNodeText() {	if (_text) delete [] _text; }
};


// ----------------------------------------------------------------------------
class cNodeHeading : public cNodeText
{
public:
	char type;

public:
	cNodeHeading() : cNodeText(INFO_NODEHEADING) {}
};

// ----------------------------------------------------------------------------
class cNodeMenu : public cNodeLine
{
public:
	char *node;
	char *name;
	char *desc;

public:
	cNodeMenu() : cNodeLine(INFO_NODEMENU) {	node = name = desc = NULL; }
	virtual ~cNodeMenu()
	{
		if (node) delete [] node;
		if (name) delete [] name;
		if (desc) delete [] desc;
	}
};

// ----------------------------------------------------------------------------
class cNodeMenuCont : public cNodeText
{
public:
	cNodeMenuCont() : cNodeText(INFO_NODEMENUCONT) { }
};


// ----------------------------------------------------------------------------
class cNodeXRef : public cNodeLine
{
public:
	char *node;
	char *name;
	char *desc;

public:
	cNodeXRef() : cNodeLine(INFO_NODEXREF) {	node = name = desc = NULL; }
	virtual ~cNodeXRef()
	{
		if (node) delete [] node;
		if (name) delete [] name;
		if (desc) delete [] desc;
	}
};

// ----------------------------------------------------------------------------
class cNodeLineList
{
private:
	cNodeLine *head;
	cNodeLine *tail;
	cNodeLine *curr;

public:
	cNodeLineList();
	~cNodeLineList();

	void Reset();
	int  Read(ifstream &stream);
	char CheckUnderline(const char *buf1, const char *buf2);
	void Add(cNodeLine *nodeLine);
	void AddText(const char *text);
	void AddInlineText(const char *text);
	void AddMenu(const char *buffer);
	void AddMenuCont(const char *text);
	void AddXRef(ifstream &stream, char *buffer);
	void AddLongText(char *text, int &pos, int indentSize = 0);
	void AddHeading(const char *text, int type);
	cNodeLine *GotoHead()	{	return curr = head; }
	cNodeLine *Next()		{	return curr = curr->next; }
	cNodeLine *Get()		{	return curr; }
	void Print();
};

// ----------------------------------------------------------------------------
class cNode
{
public:
	char	*file;
	char	*node;
	char	*next;
	char	*prev;
	char	*up;
	char 	pos[256];
	char 	findSubstring;
	char	findCase;
	char	*findString;
	cNodeLineList	nodeLines;

public:
	cNode();
	~cNode();

	void Reset();
	int  Read(const char *filename, int offset);
	char *GetPositionString();
	void initFind(const char *string, char caseSens, char subString);
	int  findNext();
	int  findText(const char *testString);
	void Print();
};

// ============================================================================
// Info class - basically runs the show

class cInfo : public cHelpFormatBase
{
private:
	cIndirectList	indirectList;
	char 			filename[256];
	char			infoFile[256];
	char			decompressCmd[2][256];
	char 			compressExtn[2][10];
	char			*searchPath[INFO_MAXPATHS];
	char			*currentPath; // will be set to last searchPath entry
	int				numPaths;
	char			compressed;
	char			firstFind;

public:
	cTagTable		tagTable;
	cNode			node;

private:
	int  FindFile(const char *theFilename);
	void Decompress(const char *theFilename, char *workFile);

public:
	cInfo();
	virtual ~cInfo();

	virtual void Reset();
	virtual int  ReadLocation(const char *nodeName);
	virtual const char *GetTitle();
	virtual const char *GetLocation()	{	return node.GetPositionString(); }

	virtual const char *PrevNode()	{	return node.prev; }
	virtual const char *NextNode()	{	return node.next; }
	virtual const char *UpNode()	{	return node.up; }
	virtual const char *TopNode()	{	return "Top"; }
	virtual const char *Dir()		{	return "(dir)"; }
	virtual int  IsTop()	{	return strcmp(node.node, "Top"); }
	virtual int  IsDir()	{	return strcmp(node.file, "dir"); }

	int  Read(const char *theFilename);
	void Print();
	void initFind(const char *string, char caseSens, char subString);
	int  findNext();
};


// ============================================================================
// misc functions

int FindMarker(ifstream &stream);

#endif
