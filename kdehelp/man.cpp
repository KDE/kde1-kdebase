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
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <dirent.h>
#include <kconfig.h>
#include <kapp.h>
#include "misc.h"
#include "man.h"
#include "error.h"
#include <errno.h>

#include "dbnew.h"

#include <klocale.h>

#ifdef HAVE_PATHS_H
#include <paths.h>
#endif

#ifndef _PATH_TMP
#define _PATH_TMP "/tmp"
#endif

#define MAXSECTIONLEN	4

// ============================================================================
// this is an array of sections available
// the first instance of cMan creates these sections
// as a section is entered the contents are read (once only).
//
static cManSection	*sections[MAN_MAXSECTIONS];
static int			numSections = 0;

// ----------------------------------------------------------------------------
// creates a menu of available sections
//
int MakeDirectory( QString &HTMLPage )
{
	char text[80];
	char page[80];

	HTMLPage = "<H1>";
	HTMLPage += klocale->translate("Online Manuals");
	HTMLPage += "</H1>";

    HTMLPage += "<dl>";

	for (int i = 0; i < numSections; i++)
	{
		sprintf(text, klocale->translate("Section %s"), sections[i]->GetName());
		sprintf(page, "(%s)", sections[i]->GetName());
		HTMLPage += "<dt><A HREF=man:";
		HTMLPage += page;
		HTMLPage += ">";
		HTMLPage += QString( text );
		HTMLPage += "</A><dd>";
        HTMLPage += sections[i]->GetDesc();
	}
    HTMLPage += "</dl>";

	return 0;
}

// ----------------------------------------------------------------------------
// creates a list of available pages in a section
//
int Read(cManSection &sect, QString &page )
{
	char buffer[256];
	sect.MoveToHead();

	sprintf(buffer, klocale->translate("Online manual - Section %s"), sect.GetName());

	page = "<H1>" + QString( buffer ) + "</H1>";

	while (sect.Get())
	{
		sprintf(buffer, "%s(%s)", sect.Get()->name(), sect.GetName());
        page += "<cell width=200>&nbsp;";
		page += "<A HREF=man:" + QString( buffer ) + ">";
		page += sect.Get()->name();
		page += "</A>";
		page += "</cell>";
		sect.Next();
	}

	return 0;
}


// ============================================================================
// section stuff - maintain a list of man pages available in a section

char *cManSection::searchPath[MAN_MAXPATHS];
int cManSection::numPaths = 0;
int cManSection::sectCount = 0;

// ----------------------------------------------------------------------------
// constructor
//
cManSection::cManSection(const char *theName)
{
	char *envPath;
	name = StrDup(theName);
	numPages = 0;
	isRead = 0;
	head = tail = NULL;

    sectCount++;

	KConfig *config = KApplication::getKApplication()->getConfig();
	config->setGroup( "ManSections" );

	QString ename = "man";
	ename += theName;
	QString sdesc = config->readEntry( ename );
	if ( sdesc.isNull() )
	{
		if ( strncmp( theName, "1", 1 ) == 0 )
			sdesc = klocale->translate("User Commands");
		else if ( strncmp( theName, "2", 1 ) == 0 )
			sdesc = klocale->translate("System Calls");
		else if ( strncmp( theName, "3", 1 ) == 0 )
			sdesc = klocale->translate("Subroutines");
		else if ( strncmp( theName, "4", 1 ) == 0 )
			sdesc = klocale->translate("Devices");
		else if ( strncmp( theName, "5", 1 ) == 0 )
			sdesc = klocale->translate("File Formats");
		else if ( strncmp( theName, "6", 1 ) == 0 )
			sdesc = klocale->translate("Games");
		else if ( strncmp( theName, "7", 1 ) == 0 )
			sdesc = klocale->translate("Miscellaneous");
		else if ( strncmp( theName, "8", 1 ) == 0 )
			sdesc = klocale->translate("System Administration");
		else if ( strncmp( theName, "n", 1 ) == 0 )
			sdesc = klocale->translate("New");
		else
			sdesc = "";

		config->writeEntry( ename, sdesc );
	}

	strcpy( desc, sdesc );

    if ( numPaths == 0 )
    {
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
#ifndef __FreeBSD__
            searchPath[numPaths++] = StrDup("/usr/man");
#else
            searchPath[numPaths++] = StrDup(_PATH_MAN);
            searchPath[numPaths++] = StrDup("/usr/X11R6/man");
#endif
            searchPath[numPaths++] = StrDup("/usr/local/man");
        }
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

    sectCount--;

    if ( sectCount == 0 )
    {
        for (i = 0; i < numPaths; i++)
            delete [] (searchPath[i]);
        numPaths = 0;
    }

	curr = head;
	while (curr)
	{
		tmp = curr;
		curr = curr->next();
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
void cManSection::ReadDir(const char *dirName )
{
	DIR *dir;
	struct dirent *dirEntry;
	char *ptr;
	char buffer[256];

	cout << klocale->translate("Reading Dir: ") << dirName << endl;

	dir = opendir(dirName);

    if ( dir )
    {
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
		result = strcmp(head->name(), pageName);
		if ( result > 0 )
		{
			page = new cPage(pageName);
			page->setNext( head );
			head = page;
			return;
		}
		else if ( result == 0 )
			return;

		while ( curr->next() )
		{
			int result = strcmp( curr->next()->name(), pageName );
			if ( result < 0 )
				curr = curr->next();
			else if ( result == 0)
				return;
			else
			{
				page = new cPage( pageName );
				tmp = curr->next();
				curr->setNext( page );
				page->setNext( tmp );
				return;
			}
		}
		page = new cPage( pageName );
		curr->setNext( page );
	}
	else
	{
		page = new cPage( pageName );
		head = tail = page;
	}
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
	instance--;
	if (instance == 0)
	{
		for (int i = 0; i < numSections; i++)
			delete sections[i];
	}
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
				sections[i]->ReadSection();
				Read(*(sections[i]), HTMLPage);
				strcpy(posString, klocale->translate("Online Manual - Section"));
				strcat(posString, sections[i]->GetName());
				pos = MAN_POSSECTION;
				return 0;
			}
		}

		if ( !strcmp(tmpName+1, MAN_DIRECTORY)) // get a list of sections
		{
			MakeDirectory( HTMLPage );
			strcpy(posString, klocale->translate("Online Manuals"));
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
		char rmanCmd[256];
		char *ptr;

        sysCmd[0] = '\0';

		sprintf(stdFile, "%s/khelpXXXXXX", _PATH_TMP);	// temp file
		mktemp(stdFile);

		sprintf(errFile, "%s/khelpXXXXXX", _PATH_TMP);	// temp file
		mktemp(errFile);

		sprintf(rmanCmd, "%s/rman -f HTML",
			(const char *)KApplication::kde_bindir());

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
            if ( safeCommand( sections[pos]->GetName() ) &&
                safeCommand( tmpName ) )
            {
                sprintf(sysCmd, "man %s %s < /dev/null 2> %s | %s > %s",
                    sections[pos]->GetName(),
                    tmpName, errFile, rmanCmd, stdFile );
            }
		}
		else if ( safeCommand( tmpName ) )
		{
			sprintf(sysCmd, "man %s < /dev/null 2> %s | %s > %s",
				tmpName, errFile, rmanCmd, stdFile);
		}

        if ( sysCmd[0] == '\0' )
        {
			Error.Set(ERR_WARNING, klocale->translate("\"man\" system call failed"));
            return -1;
        }

		// call 'man' to read man page
		int status = system(sysCmd);

		if (status < 0)			// system returns -ve on failure
		{
			Error.Set(ERR_WARNING, klocale->translate("\"man\" system call failed"));
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
			Error.Set(ERR_FATAL, klocale->translate("Opening temporary file failed"));
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

		char buffer[256];
		HTMLPage = "";

		while ( !stream.eof() )
		{
		    stream.getline( buffer, 256 );
		    HTMLPage += buffer;
            if ( HTMLPage.right(1) == "-" )
                HTMLPage.truncate( HTMLPage.length() - 1 );
            else
                HTMLPage.append(" ");
		}

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

