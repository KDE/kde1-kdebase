// ============================================================================
// man page parser
//
// let 'man' do all the hard work - just get list of man pages
// and parse man's output.
//
// Copyright (C) Martin R. Jones 1995
//
// ============================================================================

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/wait.h>
#include <Kconfig.h>
#include <kapp.h>
#include "misc.h"
#include "man.h"
#include "error.h"
#include <errno.h>

#include "dbnew.h"

#define MAXSECTIONLEN	4
#define TMPDIR	"/tmp"

// ============================================================================
// this is an array of sections available
// the first instance of cMan creates these sections
// as a section is entered the contents are read (once only).
//
static cManSection	*sections[MAN_MAXSECTIONS];
static int			numSections = 0;

// ============================================================================
// cManTextList - list of text, xrefs etc. derived from man page

// ----------------------------------------------------------------------------
// constructor
//
cManTextList::cManTextList()
{
	head = tail = curr = NULL;
}

// ----------------------------------------------------------------------------
// destructor
//
cManTextList::~cManTextList()
{
	Reset();
}

// ----------------------------------------------------------------------------
// free list
//
void cManTextList::Reset()
{
	cManBase *tmp;
	curr = head;

	while (curr)
	{
		tmp = curr;
		curr = curr->next;
		delete tmp;
	}

	head = tail = curr = NULL;
}

// ----------------------------------------------------------------------------
// creates a menu of available sections
//
int cManTextList::MakeDirectory()
{
	char text[80];
	char page[80];

	AddText("Online Manuals", MAN_HEADING1);
	AddCR();
	AddCR();

	for (int i = 0; i < numSections; i++)
	{
		sprintf(text, "Section %s", sections[i]->GetName());
		sprintf(page, "(%s)", sections[i]->GetName());
		AddDir(text, page, sections[i]->GetDesc(), MAN_DIRBOOK);
	}

	return 0;
}

// ----------------------------------------------------------------------------
// creates a list of available pages in a section
//
int cManTextList::Read(cManSection &sect)
{
	char buffer[256];
	sect.MoveToHead();

	sprintf(buffer, "Online manual - Section %s", sect.GetName());
	AddText(buffer, MAN_HEADING1);
	AddCR();
	AddCR();

	while (sect.Get())
	{
		sprintf(buffer, "%s(%s)", sect.Get()->name, sect.GetName());
		AddDir(sect.Get()->name, buffer, NULL, MAN_DIRPAGE);
		sect.Next();
	}

	return 0;
}

// ----------------------------------------------------------------------------
// read and parse a man page
//
int cManTextList::Read(ifstream &stream)
{
	char buffer[1024];
	char text[256], header[256];
	char workText[256];
	const char *ptr, *xRef, *chPos;
	char mode, getXRefs = 1;
	int i;

	// Find the header - first line of text
	while (!stream.eof())
	{
		stream.getline(buffer, 1023);
		if (buffer[0] == '\0') continue;

		ExtractText(buffer, text, strlen(buffer));
		strcpy(header, text);
		AddText(header, MAN_HEADING3);
		AddCR();
		break;
	}

	// read body of man page
	while (!stream.eof())
	{
		stream.getline(buffer, 1023);

		if (strchr(buffer, 27))	// escape sequence
			continue;

		ptr = buffer;
		i = 0;

		// extract text free of codes from line
		ExtractText(buffer, text, strlen(buffer));
		if (!strcmp(header, text))
		{
			RemoveFooter();
			continue;
		}

		// do we have a xref in this line
		xRef = FindXRef(text);
		chPos = text;

		// get the initial mode
		if (buffer[1] != '\b') mode = MAN_TEXT;
		else if (buffer[0] == '_') mode = MAN_UNDERLINE;
		else mode = MAN_BOLD;

		// parse a line
		while (*ptr != '\0')
		{
			if ((getXRefs) && (chPos == xRef))
			{
				workText[i] = '\0';
				AddText(workText, mode);
				i = strchr(xRef, ')')-xRef+1;
				strncpy(workText, xRef, i);
				workText[i] = '\0';
				AddXRef(workText, workText);
				xRef = FindXRef(strchr(xRef, ')') + 1);
				chPos += i;
				ptr = strchr(ptr, ')');
				if (*(ptr + 1) == '\b') ptr += 3;
				else ptr++;

				if (*(ptr+1) != '\b') mode = MAN_TEXT;
				else if (*ptr == '_') mode = MAN_UNDERLINE;
				else mode = MAN_BOLD;

				workText[i] = '\0';
				i = 0;
				if (*ptr == '\0') break;
			}

			if (*(ptr+1) != '\b')			// normal text
			{
				if (mode != MAN_TEXT)
				{
					workText[i] = '\0';
					AddText(workText, mode);
					mode = MAN_TEXT;
					i = 0;
				}

				workText[i++] = *ptr++;
				chPos++;
			}
			else if (*ptr == '_')			// underlined text
			{
				if (mode != MAN_UNDERLINE)
				{
					workText[i] = '\0';
					AddText(workText, mode);
					mode = MAN_UNDERLINE;
					i = 0;
				}

				ptr += 2;
				workText[i++] = *ptr++;
				chPos++;
			}
			else							// bold text
			{
				if (mode != MAN_BOLD)
				{
					workText[i] = '\0';
					AddText(workText, mode);
					mode = MAN_BOLD;
					i = 0;
				}

				ptr += 2;
				workText[i++] = *ptr++;
				chPos++;
			}
		}
		workText[i] = '\0';
		if (strlen(workText))
			AddText(workText, mode);
		AddCR();
	}

	return 0;
}

// ----------------------------------------------------------------------------
//
void cManTextList::ExtractText(char *in, char *out, int len)
{
	int i = 0;

	while (i < len)
	{
		if (in[i+1] == '\b')
		{
			*out++ = in[i+2];
			i += 3;
		}
		else
			*out++ = in[i++];
	}

	*out = '\0';
}

// ----------------------------------------------------------------------------
// add text/xref/dir/etc. to the list
//
void cManTextList::Add(cManBase *manBase)
{
	if (head)
	{
		manBase->prev = tail;
		tail->next = manBase;
		tail = manBase;
	}
	else
	{
		head = tail = manBase;
	}
}

// ----------------------------------------------------------------------------
// Remove the last line from the list.
// used to remove footer
//
void cManTextList::RemoveFooter()
{
	cManBase *tmp;

	if ( !head )
		return;
	
	while ( tail != head && tail->prev)
	{
		tmp = tail;
		tail = tail->prev;
		tail->next = NULL;

		if (tmp->type == MAN_TEXT && strlen(tmp->text) > 5)
		{
			delete tmp;
			break;
		}
		delete tmp;
	}

	while ( tail != head && tail->prev )
	{
		tmp = tail;
		tail = tail->prev;
		tail->next = NULL;
		delete tmp;

		if (tail->type != MAN_CR)
			break;
	}
}

// ----------------------------------------------------------------------------
// add a carriage return to the list
//
void cManTextList::AddCR()
{
	cManBase *manBase = new cManBase(MAN_CR);

	Add(manBase);
}

// ----------------------------------------------------------------------------
// add normal/bold/underline text to list
//
void cManTextList::AddText(const char *theText, char theMode)
{
	cManText *manText = new cManText(theMode);
	manText->text = StrDup(theText);

	Add(manText);
}

// ----------------------------------------------------------------------------
// add a xref to the list
//
void cManTextList::AddXRef(const char *theText, const char *theRef)
{
	cManXRef *manXRef = new cManXRef;

	manXRef->text = StrDup(theText);
	manXRef->page = StrDup(theRef);

	Add(manXRef);
}

// ----------------------------------------------------------------------------
// add a directory entry - i.e. menu-like entry.
//
void cManTextList::AddDir(const char *theText, const char *theRef,
	const char *theDesc, char theType)
{
	cManDir *manDir = new cManDir;

	manDir->text = StrDup(theText);
	manDir->page = StrDup(theRef);
	if (theDesc)
		manDir->desc = StrDup(theDesc);
	manDir->dirType = theType;

	Add(manDir);
}

// ----------------------------------------------------------------------------
// find a possible cross reference in the text
//
const char *cManTextList::FindXRef(const char *theText)
{
	int i, len;
	char buffer[80];
	const char *ptr, *ptr1, *xrefPtr;

	ptr = strchr(theText, '(');

	while (ptr)
	{
		ptr1 = strchr(ptr, ')');
		if (ptr1)
		{
			if ((ptr1-ptr-1 > MAXSECTIONLEN) || (ptr1-ptr <= 1))
				return NULL;

			for (i = 0; i < numSections; i++)
			{
				if (!strncmp(ptr+1, sections[i]->GetName(),
						strlen(sections[i]->GetName())))
				{
					xrefPtr = ptr-1;

					// this allows 1 space between name and '('
					if (*xrefPtr == ' ') xrefPtr--;
					if (*xrefPtr == ' ') return NULL;

					len = 1;

					while ((xrefPtr > theText) && (*(xrefPtr-1) != ' '))
					{
						xrefPtr--;
						len++;
					}

					strncpy( buffer, xrefPtr, len );
					buffer[len] = '\0';

					if ( sections[i]->FindPage( buffer ) )
						return xrefPtr;
				}
			}

			ptr = strchr(ptr1, '(');
		}
		else
			ptr = NULL;
	}

	return NULL;
}

// ============================================================================
// section stuff - maintain a list of man pages available in a section

// ----------------------------------------------------------------------------
// constructor
//
cManSection::cManSection(const char *theName)
{
	char *envPath;
	name = StrDup(theName);
	numPaths = 0;
	numPages = 0;
	isRead = 0;
	head = tail = NULL;

	KConfig *config = KApplication::getKApplication()->getConfig();
	config->setGroup( "ManSections" );

	QString ename = "man";
	ename += theName;
	QString sdesc = config->readEntry( ename );
	if ( sdesc.isNull() )
	{
		if ( strncmp( theName, "1", 1 ) == 0 )
			sdesc = "User Commands";
		else if ( strncmp( theName, "2", 1 ) == 0 )
			sdesc = "System Calls";
		else if ( strncmp( theName, "3", 1 ) == 0 )
			sdesc = "Subroutines";
		else if ( strncmp( theName, "4", 1 ) == 0 )
			sdesc = "Devices";
		else if ( strncmp( theName, "5", 1 ) == 0 )
			sdesc = "File Formats";
		else if ( strncmp( theName, "6", 1 ) == 0 )
			sdesc = "Games";
		else if ( strncmp( theName, "7", 1 ) == 0 )
			sdesc = "Miscellaneous";
		else if ( strncmp( theName, "8", 1 ) == 0 )
			sdesc = "System Administration";
		else if ( strncmp( theName, "n", 1 ) == 0 )
			sdesc = "New";
		else
			sdesc = "";

		config->writeEntry( ename, sdesc );
	}

	strcpy( desc, sdesc );

	if ( (envPath = getenv(MAN_ENV)) )
	{
		char *paths = StrDup(envPath);
		char *p = strtok(paths, ":");

		while (p)
		{
			searchPath[numPaths++] = StrDup(p);
			p = strtok(NULL, ":");
		}
	
		delete [] paths;
	}
	else
	{
		searchPath[numPaths++] = StrDup("/usr/man");
		searchPath[numPaths++] = StrDup("/usr/local/man");
	}
}

// ----------------------------------------------------------------------------
// destructor
//
cManSection::~cManSection()
{
	cPage *tmp;
	int i;

	delete [] name;

	for (i = 0; i < numPaths; i++)
		delete [] (searchPath[i]);

	curr = head;
	while (curr)
	{
		tmp = curr;
		curr = curr->next;
		delete tmp;
	}
}

// ----------------------------------------------------------------------------
// read a list of pages in the section
//
void cManSection::ReadSection()
{
	char buffer[256];
	int i;
	DIR *dir;
	struct dirent *dirEntry;

	if (!isRead)		// only read once
		isRead = 1;
	else
		return;

	for (i = 0; i < numPaths; i++)
	{
		dir = opendir(searchPath[i]);

		if (dir == NULL)
		  warning("%s: %s",searchPath[i], strerror(errno));

		else {
		while ((dirEntry = readdir(dir))) // check for matching cat and man dir
		{
			if (!strncmp(dirEntry->d_name, "man", 3))
			{
				if (!strcmp(strtok(dirEntry->d_name+3, "."), name))
				{
					sprintf(buffer, "%s/%s", searchPath[i], dirEntry->d_name);
					ReadDir(buffer);
				}
			}
			else if (!strncmp(dirEntry->d_name, "cat", 3))
			{
				if (!strcmp(strtok(dirEntry->d_name+3, "."), name))
				{
					sprintf(buffer, "%s/%s", searchPath[i], dirEntry->d_name);
					ReadDir(buffer);
				}
			}
		}

		closedir(dir);
};
	}
}

// ----------------------------------------------------------------------------
// read the contents of a directory and add to the list of pages
//
void cManSection::ReadDir(const char *dirName)
{
	DIR *dir;
	struct dirent *dirEntry;
	char *ptr;
	char buffer[256];

	cout << "Reading Dir : " << dirName << endl;

	dir = opendir(dirName);

	while ( (dirEntry = readdir(dir)) )
	{
		if ( dirEntry->d_name[0] == '.' )
			continue;

		strcpy( buffer, dirEntry->d_name );

		ptr = strrchr( buffer, '.' );
		if ( !strcmp(ptr, ".gz") || !strcmp(ptr, ".Z") ) // skip compress extn
		{
			*ptr = '\0';
			ptr = strrchr( buffer, '.' );
		}
		if (ptr)
		{
			*ptr = '\0';
			AddPage( buffer );
		}
	}

	closedir(dir);
}

// ----------------------------------------------------------------------------
// add a page to the list
//
void cManSection::AddPage(const char *pageName)
{
	cPage *page, *tmp;
	curr = head;
	int result;

	// insert page in alphabetical order
	if ( head )
	{
		result = strcmp(head->name, pageName);
		if ( result > 0 )
		{
			page = new cPage(pageName);
			page->next = head;
			head = page;
			return;
		}
		else if ( result == 0 )
			return;

		while ( curr->next )
		{
			int result = strcmp( curr->next->name, pageName );
			if ( result < 0 )
				curr = curr->next;
			else if ( result == 0)
				return;
			else
			{
				page = new cPage( pageName );
				tmp = curr->next;
				curr->next = page;
				page->next = tmp;
				return;
			}
		}
		page = new cPage( pageName );
		curr->next = page;
	}
	else
	{
		page = new cPage( pageName );
		head = tail = page;
	}
}

// ----------------------------------------------------------------------------
// find a man page
//
cPage *cManSection::FindPage( const char *pageName )
{
	ReadSection();

	cPage *page = head;

	while ( page )
	{
		if ( !strcmp( page->name, pageName ) )
		{
			printf( "Found page: %s(%s)\n", pageName, name );
			return page;
		}
		page = page->next;
	}

	printf( "Cannot find page: %s(%s)\n", pageName, name );

	return NULL;
}


// ============================================================================
// cMan - controlling class

int cMan::instance = 0;

// ----------------------------------------------------------------------------
// contructor
//
cMan::cMan() : cHelpFormatBase()
{
	char sectList[80];
	
	char *sects = getenv( "MANSECT" );
	
	if ( sects )
		strcpy( sectList, sects );
	else
		strcpy( sectList, MAN_SECTIONS );

	sectList[ strlen( sectList ) + 1 ] = '\0';


	// create the sections (this does not read the contents of a section.
	// This is done only when it is requested).
	// we do this only for the first instance
	if (instance == 0)
	{
		numSections = 0;

		// get the manual sections
		// I'd like to use strtok here, but its used in cManSection
		// constructor (strtok annoys me).
		char *s = sectList;
		char *e = strchr(sectList, ':');
		while (*s != '\0')
		{
			if (e) *e = '\0';
			sections[numSections++] = new cManSection(s);
			s += strlen(s)+1;
			e = strchr(s, ':');
		}
	}

	instance++;

	pos = MAN_POSSECTION;
//	ReadLocation(MAN_DIRECTORY);
}

// ----------------------------------------------------------------------------
// destructor
//
cMan::~cMan()
{
	Reset();
	instance--;
	if (instance == 0)
	{
		for (int i = 0; i < numSections; i++)
			delete sections[i];
	}
}

// ----------------------------------------------------------------------------
// reset everything
//
void cMan::Reset()
{
	manList.Reset();
}

// ----------------------------------------------------------------------------
// read the specified page
// formats allowed
// MAN_DIRECTORY	reads a list of sections
// (sec)			reads a list of pages in 'sections'
// page(sec)		reads the specified page from 'sections'
// page				reads the specified page
//
int cMan::ReadLocation(const char *name)
{
	char tmpName[80];
	int i = 0;

	strcpy(tmpName, name);

	if (*tmpName == '(')				// read a list of pages in this section
	{
		strtok(tmpName+1, ")");
		for (i = 0; i < numSections; i++)
		{
			if (!strcmp(tmpName+1, sections[i]->GetName()))
			{
				Reset();
				sections[i]->ReadSection();
				manList.Read(*(sections[i]));
				strcpy(posString, "Online Manual - Section ");
				strcat(posString, sections[i]->GetName());
				pos = MAN_POSSECTION;
				return 0;
			}
		}

		if ( !strcmp(tmpName+1, MAN_DIRECTORY)) // get a list of sections
		{
			Reset();
			manList.MakeDirectory();
			strcpy(posString, "Online Manuals");
			pos = MAN_POSDIR;
			return 0;
		}
		return 1;
	}
	else									// read the specified page
	{
		char stdFile[256];
		char errFile[256];
		char sysCmd[256];
		char *ptr;

		sprintf(stdFile, "%s/khelpXXXXXX", TMPDIR);	// temp file
		mktemp(stdFile);

		sprintf(errFile, "%s/khelpXXXXXX", TMPDIR);	// temp file
		mktemp(errFile);

		// create the system cmd to read the man page
		if ( (ptr = strchr(tmpName, '(')) )
		{
			if (!strchr(tmpName, ')')) return 1;	// check for closing )
			*ptr = '\0';
			ptr++;
			for (i = 0; i < numSections; i++)	// read which section?
			{
				if (!strncmp(ptr, sections[i]->GetName(),
					strlen(sections[i]->GetName())))
				{
					pos = i;
					break;
				}
			}
			sprintf(sysCmd, "man %s %s < /dev/null > %s 2> %s", sections[pos]->GetName(),
				tmpName, stdFile, errFile);
		}
		else
		{
			sprintf(sysCmd, "man %s < /dev/null > %s 2> %s", tmpName, stdFile, errFile);
		}

		// call 'man' to read man page
		int status = system(sysCmd);

		if (status < 0)			// system returns -ve on failure
		{
			Error.Set(ERR_WARNING, "\"man\" system call failed");
			return 1;
		}

/*
		if (WEXITSTATUS(status) != MAN_SUCCESS)
		{
			char msg[256];
			sprintf(msg, "Cannot open man page: %s", name);
			Error.Set(ERR_WARNING, msg);
			return 1;
		}
*/
		// open the man page and parse it
		ifstream stream(stdFile);

		if (stream.fail())
		{
			Error.Set(ERR_FATAL, "Opening temporary file failed");
			return 1;
		}

		// if this file is very short assume the man call failed
		stream.seekg( 0, ios::end );
		if ( stream.tellg() < 5 )
		{
			stream.close();
			stream.open( errFile );
		}
		stream.seekg( 0, ios::beg );

		Reset();
		manList.Read(stream);
		stream.close();
		strcpy(posString, name);

		remove(stdFile);	// don't need tmp file anymore
		remove(errFile);	// don't need tmp file anymore
	}

	return 0;
}

// ----------------------------------------------------------------------------
const char *cMan::PrevNode()
{
	if (pos == MAN_POSDIR)
		return NULL;

	if (pos == MAN_POSSECTION)
	{
		strcpy(staticBuffer, "(");
		strcat(staticBuffer, MAN_DIRECTORY);
		strcat(staticBuffer, ")");
	}
	else
	{
		strcpy(staticBuffer, "(");
		strcat(staticBuffer, sections[pos]->GetName());
		strcat(staticBuffer, ")");
	}

	return staticBuffer;
}

// ----------------------------------------------------------------------------
const char *cMan::TopNode()
{
	if ((pos == MAN_POSDIR) || (pos == MAN_POSSECTION))
		return NULL;

	strcpy(staticBuffer, "(");
	strcat(staticBuffer, sections[pos]->GetName());
	strcat(staticBuffer, ")");

	return staticBuffer;
}

