// ----------------------------------------------------------------------------
// Info file parser classes
//
// classes for reading texinfo info files
//
// Copyright (C) Martin R. Jones 1995
//
// ----------------------------------------------------------------------------

#ifdef __FreeBSD__
# define DEFAULT_INFOPATH "/usr/local/info:/usr/info:/usr/local/lib/info:/usr/lib/info:/usr/local/gnu/info:/usr/local/gnu/lib/info:/usr/gnu/info:/usr/gnu/lib/info:/opt/gnu/info:/usr/share/info:/usr/share/lib/info:/usr/local/share/info:/usr/local/share/lib/info:/usr/gnu/lib/emacs/info:/usr/local/gnu/lib/emacs/info:/usr/local/lib/emacs/info:/usr/local/emacs/info:."
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream.h>
#include <fstream.h>
#include <unistd.h>
#include <ctype.h>
#include "misc.h"
#include "info.h"
#include "error.h"

#include "dbnew.h"

#include <klocale.h>
#include <kapp.h>

#ifdef HAVE_PATHS_H
#include <paths.h>
#endif

#ifndef _PATH_TMP
#define _PATH_TMP "/tmp"
#endif

#define EATSPACE(ptr)	while ((*ptr == ' ')||(*ptr == '\t')) ptr++;

// ============================================================================
// cIndirect member functions

cIndirect::cIndirect( const char *f, int off )
{
    mFile = StrDup( f );
    mOffset = off;
    mNext = 0;
}

cIndirect::~cIndirect()
{
    delete [] mFile;
}


// ============================================================================
// cIndirectList member functions

// ----------------------------------------------------------------------------
// constructor
//
cIndirectList::cIndirectList()
{
	head = tail = 0;
}

// ----------------------------------------------------------------------------
// free mem held by list
//
cIndirectList::~cIndirectList()
{
	Reset();
}

// ----------------------------------------------------------------------------
// free mem and reset stuff
//
void cIndirectList::Reset()
{
	cIndirect *tmp, *curr = head;

	while (curr)
	{
		tmp = curr;
		curr = curr->next();
		delete tmp;
	}

	head = tail = 0;
}

// ----------------------------------------------------------------------------
// read the indirect files from the file
//
int cIndirectList::Read(const char *filename)
{
	char *file, *posPtr;
	char buffer[256];
	ifstream stream(filename);

	while (FindMarker(stream))
	{
		stream.getline(buffer, 255);
		if (!strcmp(buffer, INFO_INDIRECT_TOKEN))
		{
			do							// read the indirects
			{
				stream.getline(buffer, 255);
				if (buffer[0] == INFO_MARKER) continue;
				file = strtok(buffer, ":");
				if (file == 0)
				{
					Error.Set(ERR_FATAL, klocale->translate("Error in info file"));
					return 1;
				}
				posPtr = strtok(0, ".");
                if ( posPtr )
                {
                    Add(file, atoi(posPtr));
                }
			}
			while ((buffer[0] != INFO_MARKER) && (!stream.eof()));

			return 0;					// we have our indirects
		}
	}

	// no indirects in file - Add this file
	Add(filename, 0);

	return 0;
}

// ----------------------------------------------------------------------------
// add an indirect file to the list
//
void cIndirectList::Add(const char *file, int offset)
{
	cIndirect *indirect = new cIndirect( file, offset );

	if (head)
	{
		tail->setNext( indirect );
		tail = indirect;
	}
	else
	{
		head = tail = indirect;
	}
}

// ----------------------------------------------------------------------------
// find the node at 'offset'
//
const char *cIndirectList::Find(int &offset)
{
	cIndirect *curr = head, *prev = head;
	int fileOffset = 0;

	while (curr)
	{
		if (curr->offset() > offset) break;
		fileOffset = curr->offset();
		prev = curr;
		curr = curr->next();
	}

	offset = offset - fileOffset + head->offset();

	return prev->file();
}

// ----------------------------------------------------------------------------
// print list to stdout - debugging
//
void cIndirectList::Print()
{
	cIndirect *curr = head;

	while (curr)
	{
		cout << "File : " << curr->file() << ", Pos : " << curr->offset()
             << endl;
		curr = curr->next();
	}
}

// ============================================================================
// cTag member functions

cTag::cTag( const char *n, int off )
{
    mNode = StrDup( n );
    mOffset = off;
    mNext = 0;
}

cTag::~cTag()
{
    delete [] mNode;
}

// ============================================================================
// cTagTable member functions

// ----------------------------------------------------------------------------
// constructor
//
cTagTable::cTagTable()
{
	head = tail = 0;
}

// ----------------------------------------------------------------------------
// free tag list
//
cTagTable::~cTagTable()
{
	Reset();
}

// ----------------------------------------------------------------------------
// free mem and reset stuff
//
void cTagTable::Reset()
{
	cTag *tmp, *current = head;

	while (current)
	{
		tmp = current;
		current = current->next();
		delete tmp;
	}

	head = tail = 0;
}

// ----------------------------------------------------------------------------
// read the tag table from the file
//
void cTagTable::Read(const char *filename)
{
	char buffer[256];
	char *token;
    char haveTable = 0;
	char *file;
    char *node = 0;
    char *offsetPtr = 0;
	ifstream stream(filename);

	while (FindMarker(stream))
	{
		stream.getline(buffer, 256);
		if (!strcmp(buffer, INFO_TAGTABLE_TOKEN))
		{
			do							// read the tag table
			{
				haveTable = 1;
				file = 0;
                node = 0;
				stream.getline(buffer, 256);
				if (!strcmp(buffer, "(Indirect)")) continue;
				if (buffer[0] == INFO_MARKER) break;

				token = strtok(buffer, ":");
				EATSPACE(token);
/*
				if (!strcmp(token, INFO_FILE_TOKEN))
				{
					token = strtok(0, ",");
					EATSPACE(token);
					file = token;
					token = strtok(0, ":");
					EATSPACE(token);
				}
*/
				if (!strcmp(token, INFO_NODE_TOKEN))
				{
					token = strtok(0, INFO_TAGSEPARATOR);
					EATSPACE(token);
					node = token;
					token = strtok(0, ".");
					offsetPtr = token;
				}

                if ( node && offsetPtr )
                {
                    Add(node, atoi(offsetPtr));
                }
			}
			while ((buffer[0] != INFO_MARKER) && (!stream.eof()));
		}

		if (!strcmp(buffer, INFO_TAGTABLEEND_TOKEN))
		{
			return;					// we have our tag table
		}
	}

	stream.close();

	if (!haveTable)
		CreateTable(filename);
}

// ----------------------------------------------------------------------------
// find all the nodes and build our own tag table
//
void cTagTable::CreateTable(const char *filename)
{
	int offset;
	char buffer[256];
	char *token;
	char *file;
    char *node = 0;
	ifstream stream(filename);

	if (stream.fail())
	{
		cout << klocale->translate("Cannot open: ") << filename << endl;
		exit(1);
	}

	while (FindMarker(stream))
	{
		offset = stream.tellg() - 2;
		file = 0;
        node = 0;
		stream.getline(buffer, 255);

		token = strtok(buffer, ":");
		if (!token) continue;
		EATSPACE(token);

		if (!strcmp(token, INFO_FILE_TOKEN))
		{
			token = strtok(0, "\t,");
			if (!token) continue;
			EATSPACE(token);
			file = token;
			token = strtok(0, ":");
			if (!token) continue;
			EATSPACE(token);
		}
		if (!strcmp(token, INFO_NODE_TOKEN))
		{
			token = strtok(0, "\t,");
			if (!token) continue;
			EATSPACE(token);
			node = token;
		}
		if (node)
			Add(node, offset);
	}
}

// ----------------------------------------------------------------------------
// add a tag to the tag table
//
void cTagTable::Add(char *node, int offset)
{
	cTag *tag = new cTag( node, offset );

	if (head)
	{
		tail->setNext( tag );
		tail = tag;
	}
	else
	{
		head = tail = tag;
	}
}

// ----------------------------------------------------------------------------
// search for node in the tag list and return offset if found, else -ve.
//
int cTagTable::Find(char *node)
{
	cTag *current = head;

	while (current)
	{
		if (!strcasecmp(current->node(), node))
			return current->offset();
		current = current->next();
	}

	return -1;
}

// ----------------------------------------------------------------------------
// print tag table to stdout - debugging
//
void cTagTable::Print()
{
	cTag *current = head;

	while (current)
	{
		cout << "Node : " << current->node()
			 << ", Offset : " << current->offset() << endl;
		current = current->next();
	}
}

// ============================================================================
// cNodeLineList class

// ----------------------------------------------------------------------------
// constructor
//
cNodeLineList::cNodeLineList()
{
	head = tail = 0;
}


// ----------------------------------------------------------------------------
// destructor
//
cNodeLineList::~cNodeLineList()
{
	Reset();
}

// ----------------------------------------------------------------------------
// free mem and reset stuff
//
void cNodeLineList::Reset()
{
	cNodeLine *tmp;
	curr = head;

	while (curr)
	{
		tmp = curr;
		curr = curr->next;
		delete tmp;
	}

	head = tail = curr = 0;
}

// ----------------------------------------------------------------------------
// read in a node
//
int cNodeLineList::Read(ifstream &stream)
{
	int inMenu = 0, menuLine = 0;
	char buffer[256], buffer1[256];
	char *ptr, *ptr1;
	char heading;

	stream.getline(buffer, 256);

	do
	{
		ptr = buffer;
		EATSPACE(ptr);

		if (strstr(buffer, INFO_XREF_TOKEN) ||
			strstr(buffer, INFO_XREF_TOKEN1))
		{
			AddXRef(stream, buffer);
			continue;
		}
		else if (!strncmp(ptr, INFO_MENU_TOKEN, strlen(INFO_MENU_TOKEN)))
		{
			inMenu = 1;
		}
		else if ((!strncmp(ptr, "* ", 2)) && (inMenu) && (strchr(ptr, ':')))
		{
			menuLine = 1;
			ptr1 = ptr + 2;
			EATSPACE(ptr1);
			AddMenu(ptr1);
		}
		else if ((menuLine) && (ptr[0] != '\0'))
		{
			AddMenuCont(ptr);
		}
		else if (menuLine)
		{
			menuLine = 0;
			AddText(buffer);
		}
		else
		{
			if (!stream.eof()) stream.getline(buffer1, 255);
			strcat(buffer1, " ");
			if ( (heading = CheckUnderline(buffer, buffer1)) )
			{
				AddHeading(buffer, heading);
			}
			else
			{
				AddText(buffer);
				strcpy(buffer, buffer1);
				continue;
			}
		}
		if (!stream.eof())
			stream.getline(buffer, 256);
//		strcat(buffer, " ");
	}
	while ((buffer[0] != INFO_MARKER) && (!stream.eof()));

	return 0;
}

// ----------------------------------------------------------------------------
// check whether buf1 is underlined by '*', '=' or '-'.
//
char cNodeLineList::CheckUnderline(const char *buf1, const char *buf2)
{
	int i;

	if (strlen(buf1) == strlen(buf2))
	{
		if (*buf2 == '*')
		{
			i = 0;
			while (buf2[i] == '*') i++;
			if ( (i = strlen(buf2)) )
				return INFO_HEADING1;
		}
		else if (*buf2 == '=')
		{
			i = 0;
			while (buf2[i] == '=') i++;
			if ( (i = strlen(buf2)) )
				return INFO_HEADING2;
		}
		else if (*buf2 == '-')
		{
			i = 0;
			while (buf2[i] == '-') i++;
			if ( (i = strlen(buf2)) )
				return INFO_HEADING3;
		}
	}

	return 0;
}


// ----------------------------------------------------------------------------
// add text to line list
//
void cNodeLineList::Add(cNodeLine *nodeLine)
{
	if (head)
	{
		tail->next = nodeLine;
		tail = nodeLine;
	}
	else
	{
		head = tail = nodeLine;
	}
}

// ----------------------------------------------------------------------------
// add plain text
//
void cNodeLineList::AddText(const char *text)
{
	cNodeText *nodeText = new cNodeText;

	nodeText->setText(text);

	Add(nodeText);
}


// ----------------------------------------------------------------------------
// add inline text - i.e. no CR
//
void cNodeLineList::AddInlineText(const char *text)
{
	cNodeText *nodeText = new cNodeText(INFO_NODEINLINE);

	nodeText->setText(text);

	Add(nodeText);
}

// ----------------------------------------------------------------------------
// add a heading
//
void cNodeLineList::AddHeading(const char *text, int type)
{
	cNodeHeading *nodeHeading = new cNodeHeading;

	nodeHeading->setText(text);
	nodeHeading->type = type;

	Add(nodeHeading);
}

// ----------------------------------------------------------------------------
// add a menu entry
//
void cNodeLineList::AddMenu(const char *buffer)
{
	char *ptr1, *ptr2;
	cNodeMenu *nodeMenu = new cNodeMenu;

	ptr1 = strstr(buffer, "::");

	if ( ptr1 && (*(ptr1+2) == ' ' || *(ptr1+2) == '\t' || *(ptr1+2) == '\0') )
	{
		*ptr1 = '\0';
		nodeMenu->name = StrDup(buffer);
		nodeMenu->node = StrDup(buffer);
		ptr1 += 2;
	}
	else if ( (ptr1 = strchr(buffer, ':')) )
	{
		char *nodePtr;
		*ptr1 = '\0';
		nodeMenu->name = StrDup(buffer);
		ptr1++;
		EATSPACE(ptr1);
		nodePtr = ptr2 = ptr1;
		if ( *ptr1 == '(' )
		{
			while ( *ptr1 != ')' )
				ptr1++;
		}
		if ( ( ptr2 = strchr( ptr1, '.' ) ) == 0 )
			 ptr2 = ptr1+1;
		*ptr2 = '\0';
		nodeMenu->node = StrDup(nodePtr);
		ptr1 = ptr2 + 1;
	}
	else
	{
		Error.Set(ERR_WARNING, klocale->translate("Error in info file"));
		return;
	}

	EATSPACE(ptr1);
	if (*ptr1 != '\0')
		nodeMenu->desc = StrDup(ptr1);

	Add(nodeMenu);
}

// ----------------------------------------------------------------------------
// 
void cNodeLineList::AddMenuCont(const char *text)
{
	cNodeMenuCont *nodeMenuCont = new cNodeMenuCont;

	nodeMenuCont->setText(text);

	Add(nodeMenuCont);
}

// ----------------------------------------------------------------------------
// add a cross reference
//
void cNodeLineList::AddXRef(ifstream &stream, char *buffer)
{
	cNodeXRef *nodeXRef = new cNodeXRef;
	char *ptr, *ptr1, ref[80], node[80];
	int wrapped = FALSE;

	ptr = strstr(buffer, INFO_XREF_TOKEN);
	ptr1 = strstr(buffer, INFO_XREF_TOKEN1);

	if (ptr && ptr1)
		ptr = ptr < ptr1 ? ptr : ptr1;
	else if (ptr == 0)
		ptr = ptr1;

	*ptr = '\0';
	AddInlineText( buffer );

	ptr += strlen( INFO_XREF_TOKEN );
	EATSPACE( ptr );

	*ref = '\0';
	if ( ( ptr1 = strchr( ptr, ':' ) ) == 0 )
	{
		strcpy( ref, ptr );
		stream.getline( buffer, 256 );
		ptr = buffer;
		EATSPACE( ptr );
		ptr1 = strchr( ptr, ':' );
		AddInlineText( "" );
		wrapped = TRUE;
	}

    if ( !ptr1 )
    {
        return;
    }

	if ( *(ptr1+1) == ':' )
	{
		*ptr1 = '\0';
		strcat( ref, ptr );
		nodeXRef->name = StrDup(ref);
		nodeXRef->node = StrDup(ref);
		ptr = ptr1 + 2;
	}
	else
	{
		*ptr1++ = '\0';
		strcat( ref, ptr );
		nodeXRef->name = StrDup(ref);
		EATSPACE( ptr1 );
		*node = '\0';
		if ( ( ptr = strpbrk( ptr1, ",." ) ) == 0 )
		{
			strcpy( node, ptr1 );
			stream.getline( buffer, 256 );
			ptr1 = buffer;
			ptr = strpbrk( ptr1, ",." );
			AddText( "" );
		}
		*ptr = '\0';
		strcat( node, ptr1 );
		nodeXRef->node = StrDup(node);
		ptr++;
	}

	Add(nodeXRef);
	if ( wrapped && strlen( ptr ) > 5 )
		AddText( "" );

//	EATSPACE( ptr );
	memmove( buffer, ptr, strlen( ptr ) + 1 );
}

#if 0
void cNodeLineList::AddXRef(ifstream &stream, char *buffer)
{
	cNodeXRef *nodeXRef = new cNodeXRef;
	char *ptr, *buf, *ptr1;
	int pos = 0, indentSize = 0;

	tbuf[0] = '\0';
	ptr = buffer;

	while (buffer[indentSize] == ' ')
		indentSize++;

	do
	{
		strcat(tbuf, ptr);
		stream.getline(buffer, 256);
		ptr = buffer;
		EATSPACE(ptr);
	}
	while ((ptr[0] != '\0') && (buffer[0] != INFO_MARKER));

	buf = tbuf;
	ptr = strstr(buf, INFO_XREF_TOKEN);
	ptr1 = strstr(buf, INFO_XREF_TOKEN1);

	if (ptr && ptr1)
		ptr = ptr < ptr1 ? ptr : ptr1;
	else if (ptr == 0)
		ptr = ptr1;

	do
	{
		strncpy(buffer, buf, ptr-buf);
		buffer[ptr-buf] = '\0';
		strcat(buffer, "Note: ");
		AddLongText(buffer, pos, indentSize);

		ptr += strlen(INFO_XREF_TOKEN);
		EATSPACE(ptr);
		ptr1 = strtok(ptr, ":");
		if (*(ptr1+strlen(ptr1)+1) == ':')
		{
			EATSPACE(ptr1);
			nodeXRef->name = new char [strlen(ptr1) + 2];
			strcpy(nodeXRef->name, ptr1);
			nodeXRef->node = StrDup(ptr1);
			buf = ptr1 + strlen(ptr1) + 2;
		}
		else
		{
			EATSPACE(ptr1);
			nodeXRef->name = new char [strlen(ptr1) + 2];
			strcpy(nodeXRef->name, ptr1);
			ptr1 = strtok(0, ",.");
			EATSPACE(ptr1);
			nodeXRef->node = StrDup(ptr1);
			buf = ptr1 + strlen(ptr1) + 1;
		}

		if (pos + strlen(nodeXRef->name) > 80)
		{
			AddText("");
			pos = 0;
		}
		Add(nodeXRef);
		pos += strlen(nodeXRef->name);

		ptr = strstr(buf, INFO_XREF_TOKEN);
		ptr1 = strstr(buf, INFO_XREF_TOKEN1);

		if (ptr && ptr1)
			ptr = ptr < ptr1 ? ptr : ptr1;
		else if (ptr == 0)
			ptr = ptr1;

		if (ptr)
			nodeXRef = new cNodeXRef;
	}
	while (ptr);

	AddLongText(buf, pos, indentSize);
	AddText("");
	AddText("");
}
#endif
// ----------------------------------------------------------------------------
// add text to the nodeline list - if the text is greater than one line
// then send multiple lines, breaking at word boundaries
//
void cNodeLineList::AddLongText(char *buffer, int &pos, int indentSize)
{
	char line[100] = "";
	char indent[80];
	const char *ptr;
	int i = 0;

	// keep leading spaces
	while (buffer[i++] == ' ')
	{
		strcat(line, " ");
		pos++;
	}

	ptr = strtok(buffer, " \t");

	for (i = 0; i < indentSize; i++)
		indent[i] = ' ';
	indent[i] = '\0';

	while (ptr)
	{
		if (pos+(int)strlen(ptr) > 80 - indentSize)
		{
			AddText(line);
			line[0] = '\0';
			strcat(line, indent);
			pos = indentSize;
		}

		pos += strlen(ptr) + 1;
		strcat(line, ptr);
		strcat(line, " ");

		ptr = strtok(0, " \t");
	}

	AddInlineText(line);
}


// ----------------------------------------------------------------------------
// debugging
//
void cNodeLineList::Print()
{
	curr = head;

	while (curr)
	{
		switch (curr->type)
		{
			case INFO_NODETEXT:
				cout << ((cNodeText *)curr)->text() << endl;
				break;

			case INFO_NODEMENU:
				cout << "Name: " << ((cNodeMenu *)curr)->name
					 << ", Node: " << ((cNodeMenu *)curr)->node
					 << ", desc: " << ((cNodeMenu *)curr)->desc << endl;
				break;
		}
		curr = curr->next;
	}
}

// ============================================================================
// cNode class

// ----------------------------------------------------------------------------
// destructor
//
cNode::cNode()
{
	file = node = next = prev = up = 0;
	findString = 0;
}

// ----------------------------------------------------------------------------
// destructor
//
cNode::~cNode()
{
	Reset();
}

// ----------------------------------------------------------------------------
// free mem and reset everything
//
void cNode::Reset()
{
	if (file) delete [] file;
	if (node) delete [] node;
	if (next) delete [] next;
	if (prev) delete [] prev;
	if (up)   delete [] up;

	file = node = next = prev = up = 0;

	nodeLines.Reset();
}

// ----------------------------------------------------------------------------
// read in a node
//
int cNode::Read(const char *filename, int offset)
{
	char buffer[256];
	char *token;

	Reset();

	ifstream stream(filename);
	if (stream.fail())
	{
		return 1;
	}

	// seek to a little before exact offset, then search
	offset -= 200; if (offset < 0) offset = 0;
	stream.seekg(offset);

	while (!FindMarker(stream)) if (stream.eof()) return 1;

	stream.getline(buffer, 256);
	token = strtok(buffer, ":");

	do
	{
		EATSPACE(token);
		if (!strcmp(token, INFO_FILE_TOKEN))
		{
			token = strtok(0, ",\t");
			EATSPACE(token);
			file = StrDup(token);
		}
		else if (!strcmp(token, INFO_NODE_TOKEN))
		{
			token = strtok(0, ",\t");
			EATSPACE(token);
			node = StrDup(token);
		}
		else if (!strcmp(token, INFO_NEXT_TOKEN))
		{
			token = strtok(0, ",\t");
			EATSPACE(token);
			next = StrDup(token);
		}
		else if (!strcmp(token, INFO_PREV_TOKEN))
		{
			token = strtok(0, ",\t");
			EATSPACE(token);
			prev = StrDup(token);
		}
		else if (!strcmp(token, INFO_UP_TOKEN))
		{
			token = strtok(0, ",\t");
			EATSPACE(token);
			up = StrDup(token);
		}

		token = strtok(0, ":");
	}
	while (token);

	return nodeLines.Read(stream);
}

// ----------------------------------------------------------------------------
// returns a string which contains the current position in the info file
//
char *cNode::GetPositionString()
{
	pos[0] = '\0';

	if (file)
	{
		strcat(pos, "File: ");
		strcat(pos, file);
		strcat(pos, ",  ");
	}
	if (node)
	{
		strcat(pos, "Node: ");
		strcat(pos, node);
		strcat(pos, ",  ");
	}
	if (prev)
	{
		strcat(pos, "Previous: ");
		strcat(pos, prev);
		strcat(pos, ",  ");
	}
	if (next)
	{
		strcat(pos, "Next: ");
		strcat(pos, next);
		strcat(pos, ",  ");
	}
	if (up)
	{
		strcat(pos, "Up: ");
		strcat(pos, up);
		strcat(pos, ",  ");
	}

	pos[strlen(pos)-3] = '\0';

	return pos;
}

// ----------------------------------------------------------------------------
// initialise text search
//
void cNode::initFind(const char *string, char caseSens, char subString)
{
	if (string == 0)
		return;

	findCase = caseSens;
	findSubstring = subString;

	if (findString)
		delete [] findString;

	if (findCase)
		findString = StrDup(string);
	else
		findString = StrUpperDup(string);
}

// ----------------------------------------------------------------------------
// find the next occurrance of findString
//
int cNode::findNext()
{
	if (findString == 0)
		return 1;

	char *str;
	cNodeLine *curr = nodeLines.GotoHead();

	while (curr)
	{
		switch (curr->type)
		{
			case INFO_NODETEXT:
				if (findText( ((cNodeText *)curr)->text() ))
					return 0;
				break;

			case INFO_NODEINLINE:
				if (findText( ((cNodeText *)curr)->text() ))
					return 0;
				break;

			case INFO_NODEMENU:
				if (findText( ((cNodeMenu *)curr)->name ))
					return 0;
				if ( (str = ((cNodeMenu *)curr)->desc) )
					if (findText( str ))
						return 0;
				break;

			case INFO_NODEMENUCONT:
				if (findText( ((cNodeMenuCont *)curr)->text() ))
					return 0;
				break;

			case INFO_NODEXREF:
				if (findText( ((cNodeXRef *)curr)->name ))
					return 0;
				break;

			case INFO_NODEHEADING:
				if (findText( ((cNodeHeading *)curr)->text() ))
					return 0;
				break;
		}

		curr = curr->next;
	}

	return 1;
}

// ----------------------------------------------------------------------------
// attempt to find findString in testString
//
int cNode::findText(const char *testString)
{
	const char *found;

	if (findCase)
		found = strstr(testString, findString);
	else
		found = StrUpperStr(testString, findString);

	if (found)
	{
		if (findSubstring || (!isalnum(*(found-1)) &&
				!isalnum(*(found+strlen(findString)))) )
			return 0;
	}

	return 1;
}

// ----------------------------------------------------------------------------
// debugging
//
void cNode::Print()
{
	cout << "File: " << file << ", Node: " << node
		 << " , Next: " << next << " , Prev: " << prev
		 << " , Up: " << up << endl;
	nodeLines.Print();
}

// ============================================================================
// main info class

// ----------------------------------------------------------------------------
// destructor
//
cInfo::cInfo()
{
	char *envPath=0L;

	compressed = 0;
	numPaths = 0;

	// gzip
	strcpy(decompressCmd[0], "gunzip");
	strcpy(compressExtn[0], ".gz");

	// bzip2
	strcpy(decompressCmd[1], "bunzip2");
	strcpy(compressExtn[1], ".bz2");

	// get the paths to search for info files

#ifdef DEFAULT_INFOPATH
	// Add the default ones if we have em
        char *default_paths = StrDup(DEFAULT_INFOPATH);
        char *default_p = strtok(default_paths, ":");
	while (default_p && numPaths < INFO_MAXPATHS-1)
	{
		searchPath[numPaths++] = StrDup(default_p);
		default_p = strtok(0, ":");
	}

	delete [] default_paths;

#endif
	if ( (envPath = getenv(INFO_ENV)) )
	{
		char *paths = StrDup(envPath);
		char *p = strtok(paths, ":");

		while (p && numPaths < INFO_MAXPATHS-1)
		{
			searchPath[numPaths++] = StrDup(p);
			p = strtok(0, ":");
		}

		delete [] paths;
	}
	else
		searchPath[numPaths++] = StrDup("/usr/info");

	// append space to store the last used path
	searchPath[numPaths] = new char [256];
	searchPath[numPaths][0] = '\0';
	currentPath = searchPath[numPaths];
	numPaths++;
}

// ----------------------------------------------------------------------------
// destructor
//
cInfo::~cInfo()
{
	int i;

	for (i = 0; i < numPaths; i++)
		delete [] searchPath[i];

	if (compressed)
		remove(filename);
	Reset();
}

// ----------------------------------------------------------------------------
// reset all classes
//
void cInfo::Reset()
{
	indirectList.Reset();
	tagTable.Reset();
	node.Reset();
}

// ----------------------------------------------------------------------------
const char *cInfo::GetTitle()
{
	static char title[256];

	sprintf( title, "%s: %s", node.file, node.node );

	return title;
}

// ----------------------------------------------------------------------------
// reads a node from an info file
// formats allowed are:
// "(file)node"         read the specified node from 'file'
// "(file)"             read the Top node from 'file'
// "node"               read node from the current file
//
int cInfo::ReadLocation(const char *theNodeName)
{
	int offset;
	char *file, *nodeName;
	const char *ptr1, *ptr2;

	if (!theNodeName) return 1;

	// is there a filename in the node name
	if ((theNodeName[0] == '(') &&
		(ptr2 = strrchr(theNodeName, ')')))
	{
		// extract the filename
		ptr1 = theNodeName+1;
		int len = ptr2 - ptr1;
		file = new char [len+1];
		strncpy(file, ptr1, len);
		file[len] = '\0';
		if (!strcmp(file, "DIR")) strcpy(file, "dir");
		if (Read(file))
		{
			delete [] file;
			return 1;
		}
		delete [] file;

		// extract the node
		ptr2++;
		EATSPACE( ptr2 );
		if ( *ptr2 != '\0' )
			nodeName = StrDup(ptr2);
		else
			nodeName = StrDup("Top");
	}
	else
	{
		nodeName = StrDup(theNodeName);
	}

	// Read the node
	offset = tagTable.Find(nodeName);		// get the offset into the file(s)
	if (offset < 0)
	{
		char msg[128];
		sprintf(msg, i18n("Node not found: %s"), nodeName);
		Error.Set(ERR_WARNING, msg);
		delete [] nodeName;
		return 1;
	}

	file = StrDup(indirectList.Find(offset));	// which file?

	// if the file is not currently open - open it.
	if (strcmp(infoFile, file) && strcmp(filename, file))
		if (FindFile(file)) return 1;

	delete [] file;
	delete [] nodeName;

	return node.Read(filename, offset);          // read the node
}

// ----------------------------------------------------------------------------
// reads a new info file
//
int cInfo::Read(const char *theFilename)
{
//	cout << "Reading : " << theFilename << endl;

	if (FindFile(theFilename))
		return 1;

	Reset();
	indirectList.Read(filename);
//	indirectList.Print();
	tagTable.Read(filename);
//	tagTable.Print();

	return 0;
}

// ----------------------------------------------------------------------------
// find the file we want to read and decompress if necessary
//
int cInfo::FindFile(const char *theFilename)
{
	char *ptr;
	char fullPath[256];
	char workFile[256];
	int n = 0;

	if (theFilename[0] == '(')
	{
		if (!strcmp(theFilename, "(DIR)"))
			strcpy(workFile, "dir");
		else
		{
			strcpy(workFile, theFilename+1);
			workFile[strlen(theFilename)-2] = '\0';
		}
	}
	else
		strcpy(workFile, theFilename);

	if ((workFile[0] != '.') && (workFile[0] != '/'))
	{
		while (n < numPaths)
		{
			if ( searchPath[n][0] == '\0' )
			{
				n++;
				continue;
			}
			sprintf(fullPath, "%s/%s", searchPath[n], workFile);
			if (!access(fullPath, R_OK))
			{
				bool success = false;
			  
				for (int i=0; i<2; ++i)
				{
					if (!strcmp(fullPath+strlen(fullPath)-strlen(compressExtn[i]),
						    compressExtn[i]))
					{
						Decompress(fullPath, workFile);
						goto COMPRESSED;
					}
				}
			  
				if (!success)
				{
					strcpy(workFile, fullPath);
					goto NOTCOMPRESSED;
				}
			}
			
			for (int i=0; i<2; ++i)
			{
				sprintf(fullPath, "%s/%s%s", searchPath[n], workFile,
					compressExtn[i]);
		     
				if (!access(fullPath, R_OK))
				{
					Decompress(fullPath, workFile);
					goto COMPRESSED;
				}
			}
			
			sprintf(fullPath, "%s/%s.info", searchPath[n], workFile);
			if (!access(fullPath, R_OK))
			{
				strcpy(workFile, fullPath);
				goto NOTCOMPRESSED;
			}

			for (int i=0; i<2; ++i)
			{
				sprintf(fullPath, "%s/%s.info%s", searchPath[n], workFile,
					compressExtn[i]);
				if (!access(fullPath, R_OK))
				{
					Decompress(fullPath, workFile);
					goto COMPRESSED;
				}
			}

			n++;
		}
	}
	else
	{
		if (!access(workFile, R_OK))
		{
			for (int i=0; i<2; ++i)
			{
				strcpy( fullPath, workFile );
				if (!strcmp(workFile+strlen(workFile)-strlen(compressExtn[i]),
					    compressExtn[i]))
				{
					Decompress(workFile, workFile);
					goto COMPRESSED;
				}
			}
		    
			goto NOTCOMPRESSED;
		}

		for (int i=0; i<2; ++i)
		{
			// SuSE Linux distribution: (dir) uses full paths without .gz extn
			sprintf( fullPath, "%s%s", workFile, compressExtn[i]);
			if (!access(fullPath, R_OK))
			{
				Decompress(fullPath, workFile);
				goto COMPRESSED;
			}
		}

		sprintf(fullPath, "%s.info", workFile);
		if (!access(fullPath, R_OK))
		{
			strcpy(workFile, fullPath);
			goto NOTCOMPRESSED;
		}
		
		for (int i=0; i<2; ++i)
		{
			sprintf(fullPath, "%s.info%s", workFile, compressExtn[i]);
			if (!access(fullPath, R_OK))
			{
				Decompress(fullPath, workFile);
				goto COMPRESSED;
			}
		}
	}

	char msg[256];
	sprintf(msg, klocale->translate("Cannot open info file: %s"), theFilename);
	Error.Set(ERR_WARNING, msg);
	return 1;

NOTCOMPRESSED:
	if (compressed)
		remove(filename);
	compressed = 0;

	strcpy(filename, workFile);
	strcpy(infoFile, theFilename);

	// save the full path of this file
	strcpy( currentPath, fullPath );
	ptr = strrchr( currentPath, '/' );
	if ( ptr )
		*ptr = '\0';

	return 0;

COMPRESSED:
	if (compressed)
		remove(filename);
	compressed = 1;

	strcpy(filename, workFile);
	strcpy(infoFile, theFilename);

	// save the full path of this file
	strcpy( currentPath, fullPath );
	ptr = strrchr( currentPath, '/' );
	if ( ptr )
		*ptr = '\0';

	return 0;
}

// ----------------------------------------------------------------------------
// decompress the file
//
void cInfo::Decompress(const char *theFilename, char *workFile)
{
	char tmpFile[256];
	char sysCmd[512];

	sprintf(tmpFile, "%s/khelpXXXXXX", _PATH_TMP );
	mktemp(tmpFile);
  
	int i=0;
	for (; i<2; ++i)
	{
		if (!strcmp(theFilename+strlen(theFilename)-strlen(compressExtn[i]),
			    compressExtn[i]))
			break;
	}
  
	sprintf(sysCmd, "%s < %s > %s", decompressCmd[i], theFilename, tmpFile);
	if ( safeCommand( theFilename ) )
		system(sysCmd);
  
	strcpy(workFile, tmpFile);
}

// ----------------------------------------------------------------------------
// initialise a document wide search
//
void cInfo::initFind(const char *string, char caseSens, char subString)
{
	node.initFind(string, caseSens, subString);
	firstFind = 1;
}
  
// ----------------------------------------------------------------------------
// find the next match in a document wide search
//
int cInfo::findNext()
{
	if (firstFind)
	{
		firstFind = 0;
		tagTable.GotoHead();
	}
	else if (tagTable.Get())
		tagTable.Next();
	else
		return 1;

	while (tagTable.Get())
	{
		ReadLocation(tagTable.Get()->node());

		if (!node.findNext())
		{
			return 0;
		}

		tagTable.Next();
	}

	return 1;
}

// ----------------------------------------------------------------------------
// debugging
//
void cInfo::Print()
{
	node.Print();
}

// ============================================================================
// misc functions

// find the next ^_ in the stream
int FindMarker(ifstream &stream)
{
	char buffer[256];

	do
	{
		stream.getline(buffer, 255);
		if (buffer[0] == INFO_MARKER)
			return 1;
	}
	while (!stream.eof());

	return 0;
}

