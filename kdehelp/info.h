// Info file parser classes
//
// classes for reading texinfo info files
//
// Copyright (C) Martin R. Jones 1995
//

#ifndef __INFO_H__
#define __INFO_H__

#include <stdlib.h>
#include <fstream.h>
#include "fmtbase.h"
#include "dbnew.h"

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
struct sIndirect
{
	char file[256];
	int offset;
	sIndirect *next;
};

// ----------------------------------------------------------------------------
class cIndirectList
{
private:
	sIndirect *head;
	sIndirect *tail;

public:
	cIndirectList();
	~cIndirectList();

	void Reset();
	int  Read(const char *filename);
	void Add(const char *name, int offset);
	char *Find(int &offset);
	void Print();
};

// ============================================================================
// Structures and classes used for tag table

// ----------------------------------------------------------------------------
// Tag structure - holds tag attributes
// file 	: null if no file is given (indirect?)
// node 	: node name
// offset	: offset into file (may be into indirect files)

class sTag
{
	char *_file;
public:
	char node[256];
	int offset;
	sTag *next;

	sTag() { _file = NULL; }
	~sTag() { if (_file) delete [] _file; }

	void setFile( const char *f )
	{
		if (_file)
			delete [] _file;
		_file = NULL;
		if (!f)
			return;
		_file = new char [strlen(f)+1];
		strcpy(_file, f);
	}

	const char *file()	{	return _file; }
};

// ----------------------------------------------------------------------------
// tag table class - look after tags, searching, etc.

class cTagTable
{
private:
	sTag *head;
	sTag *tail;
	sTag *curr;

public:
	cTagTable();
	~cTagTable();

	void Reset();
	void Read(const char *filename);
	void CreateTable(const char *filename);
	void Add(const char *file, char *node, int offset);
	int  Find(char *node);
	void GotoHead()		{	curr = head; }
	sTag *Get()			{	return curr; }
	void Next()			{	curr = curr->next; }
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
	char			decompressCmd[256];
	char 			compressExtn[10];
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
