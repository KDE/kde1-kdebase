/*
    This file is part of the KDE File Manager

    Copyright (C) 1998 Waldo Bastian (bastian@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License 
    as published by the Free Software Foundation; either version 2 
    of the License, or (at your option) any later version.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this library; see the file COPYING. If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
//----------------------------------------------------------------------------
//
// KDE File Manager -- HTTP Cookies
// $Id$

//
// The cookie protocol is a mess. RFC2109 is a joke since nobody seems to 
// use it. Apart from that it is badly written. 
// We try to implement Netscape Cookies and try to behave us according to
// RFC2109 as much as we can.
//
#include <qstring.h>
#include <qstrlist.h>
#include <qlist.h>
#include <qdict.h>

#include <kurl.h>

#include <stdio.h>

#include "kcookiejar.h"

// KCookie
///////////////////////////////////////////////////////////////////////////

//
// Cookie constructor
//
KCookie::KCookie(const char *_host, const char *_domain, const char *_path,
                 const char *_name, const char *_value, time_t _expireDate,
                 int _protocolVersion) :
       host(_host), 
       domain(_domain), 
       path(_path), 
       name(_name),
       value(_value),
       expireDate(_expireDate),
       protocolVersion(_protocolVersion)
{ 
    nextCookie = 0;
}

//
// Checks if a cookie has been expired
//
bool    KCookie::isExpired(time_t currentDate)
{
    return (expireDate != 0) && (expireDate < currentDate);
}

//
// Returns a string for a HTTP-header
//
QString KCookie::getCookieStr(void)
{
    QString result;
    
    if (protocolVersion != 0)
    {
        result.sprintf("$Version=\"%d\"; ", protocolVersion);
        result += name + "=\"" + value + "\"";
        if (!path.isEmpty())
            result += "; $Path=\""+ path + "\"";
        if (!domain.isEmpty())
            result += "; $Domain=\""+ domain + "\"";
    }
    else
    {
        result = name.data();
        result = result + "=" + value;
    }    
    return result;
}

// KCookieList
///////////////////////////////////////////////////////////////////////////

int KCookieList::compareItems( KCookie * item1, KCookie * item2)
{
    int pathLen1 = item1->path.length();
    int pathLen2 = item2->path.length();
    if (pathLen1 > pathLen2)
        return 1;
    if (pathLen1 < pathLen2)
        return -1;
    return 0;
}
     

// KCookieJar
///////////////////////////////////////////////////////////////////////////

//
// Constructs a new cookie jar
//
// One jar should be enough for all cookies.
// 
KCookieJar::KCookieJar()
{
    cookieDomains.setAutoDelete( true );
    globalAdvice = KCookieDunno;    
}

//
// Destructs the cookie jar
//
// Poor little cookies, they will all be eaten by the cookie monster!
//
KCookieJar::~KCookieJar()
{
    // Not much to do here
}

//
// Looks for cookies in the cookie jar which are appropriate for _url.
// Returned is a string containing all appropriate cookies in a format 
// which can be added to a HTTP-header without any additional processing.
//
QString KCookieJar::findCookies(const char *_url)
{
    QString cookieStr("");
    QString domain;
    QString fqdn;
    QString path;
    KCookiePtr cookie;
    int protVersion = 1;
    int cookieCount = 0;
    
    if (!extractDomain(_url, fqdn, domain, path))
    {
        return cookieStr;
    }

    KCookieList *cookieList = cookieDomains[domain.data()];
    
    if (!cookieList)
	return cookieStr; // No cookies for this domain    

    for ( cookie=cookieList->first(); cookie != 0; cookie=cookieList->next() )
    {
        if (!cookie->domain.isEmpty())
        {
            // Cookie has a domain set
            if (fqdn.find(cookie->domain.data()) == -1)
                continue; // Domain of cookie does not match with host of URL
	}        
	else
        {
            // Cookie has no domain set
            if (fqdn != cookie->host)
                continue; // Host of cookie does not match with host of URL
        }

        if (!cookie->path.isEmpty())
        {
            // Cookie has a path set
            if (path.find(cookie->path.data()) != 0)
                continue; // Path of URL does not start with cookie-path
	}
	// Use first cookie to determine protocol version
	if (cookieCount == 0)
	{
	   protVersion = cookie->protocolVersion;
	}
	if (protVersion == 0)
	{
	    if (cookieCount == 0)
	        cookieStr += "Cookie: ";
	    else
	        cookieStr += "; ";
            cookieStr += cookie->getCookieStr();
	}
	else 
	{
            cookieStr += "Cookie: ";
            cookieStr += cookie->getCookieStr();
            cookieStr += "\r\n";
        } 
        cookieCount++;
    }                    

    if ((protVersion == 0) && (cookieCount > 0))
        cookieStr += "\r\n";
        
    return cookieStr;
}

//
// This function parses a string like 'my_name="my_value";' and returns
// 'my_name' in Name and 'my_value' in Value.
//
// A pointer to the end of the parsed part is returned.
// This pointer points either to:
// '\0' - The end of the string has reached.
// ';'  - Another my_name="my_value" pair follows
// ','  - Another cookie follows
// '\n' - Another header follows
static const char *parseNameValue(const char *header, 
                                  QString &Name, 
                                  QString &Value)
{
    const char* s = header;

    // Skip any whitespace
    for(; (*s == ' ') || (*s == '\t'); s++)
    {           
        if ((*s=='\0') || (*s==';') || (*s=='\n'))
        {
            // End of header
            Name = "";
            Value = "";
            return (s);
        }
    }

    header = s;

    // Parse 'my_name' part
    for(; (*s != '=') && (*s != ' ') && (*s != '\t'); s++)
    {           
        if ((*s=='\0') || (*s==';') || (*s=='\n'))
        {
            // End of Name
            Value = "";
            Name = header;
            Name.truncate( s - header );
            return (s);
        }
    }
    
    Name = header;
    Name.truncate( s - header );

    // Skip any whitespace
    for(; (*s != '='); s++)
    {           
        if ((*s=='\0') || (*s==';') || (*s=='\n'))
        {
            // End of Name
            Value = "";
            return (s);
        }
    }
    
    // *s == '='
    s++;

    // Skip any whitespace
    for(; (*s == ' ') || (*s == '\t'); s++)
    {           
        if ((*s=='\0') || (*s==';') || (*s=='\n'))
        {
            // End of Name
            Value = "";
            return (s);
        }
    }
    
    if (*s == '\"')
    {
        // Parse '"my_value"' part (quoted value)
        s++;  // skip "
        header = s;
        for(;(*s != '\"');s++)
        {
            if ((*s=='\0') || (*s=='\n'))
            {
                // End of Name
                Value = header;
                Value.truncate(s - header);
                return (s);
            }
        }
        Value = header;
        Value.truncate( s - header );

        // *s == '\"';
        s++;
        // Skip any remaining garbage
        for(;; s++)
        {           
            if ((*s=='\0') || (*s==';') || (*s=='\n'))
                break;
        }
    }
    else
    {
        // Parse 'my_value' part (unquoted value)
        header = s;
        while ((*s != '\0') && (*s != ';') && (*s != '\n'))
            s++;
        // End of Name
        Value = header;
        Value.truncate( s - header );
    }
    return (s);

}    

static void stripDomain(const char *_fqdn, QString &_domain)
{
    _domain = _fqdn;
    int dot_pos = _domain.find('.');

    if (dot_pos != -1)
    {
        // Domain contains a '.': remove the hostname from the domain
        _domain.remove(0, dot_pos);
        
        if (_domain.find('.', 1) == -1)
        {
           // Domain part should contain at least another '.'
           // Use the host part instead
           _domain = _fqdn;
        }
    }

}
    
bool KCookieJar::extractDomain(const char *_url,
                               QString &_fqdn,
                               QString &_domain,
                               QString &_path)
{
    KURL kurl(_url);    
    
    if ( strcmp(kurl.protocol(), "http") != 0)
    	return false; // We only do HTTP cookies :)
    
    _fqdn = kurl.host();
    _fqdn = _fqdn.lower();
    stripDomain(_fqdn, _domain);
    _path = kurl.path();
    _path = _path.lower();
            
    int dot_pos = _domain.find('.');

    if (dot_pos != -1)
    {
        // Domain contains a '.': remove the hostname from the domain
        _domain.remove(0, dot_pos);
        
        if (_domain.find('.', 1) == -1)
        {
           // Domain part should contain at least another '.'
           // Use the host part instead
           _domain = _fqdn;
        }
    }
    return true;
}
                                                   
//
// This function parses cookie_headers and returns a linked list of
// KCookie objects for all cookies found in cookie_headers.
// If no cookies could be found 0 is returned.
//
// cookie_headers should be a concatenation of all lines of a HTTP-header
// which start with "Set-Cookie". The lines should be separated by '\n's.
// 
KCookiePtr KCookieJar::makeCookies(const char *_url, 
                                   const char *cookie_headers)
{
    KCookiePtr cookieChain = 0;
    KCookiePtr lastCookie = 0;
    const char *cookieStr = cookie_headers;
    QString Name;
    QString Value;
    QString fqdn;
    QString domain;
    QString path;
    
    if (!extractDomain(_url, fqdn, domain, path))
    {
        // Error parsing _url
        return 0;
    }
    
    //  The hard stuff :)
    for(;;)
    {
        // check for "Set-Cookie"
        if (strncasecmp(cookieStr, "Set-Cookie: ", 12) == 0)
        {
            cookieStr = parseNameValue(cookieStr+12, Name, Value);

	    if (Name.isEmpty())
	        continue;

            // Host = FQDN
            // Default domain = ""
            // Default path = ""
            lastCookie = new KCookie(fqdn, "", "", 
                                     Name.data(), Value.data() );

            // Insert cookie in chain
            lastCookie->nextCookie = cookieChain;
            cookieChain = lastCookie;
        }
        else if (lastCookie && (strncasecmp(cookieStr, "Set-Cookie2: ", 13) == 0))
        {
            // What the fuck is this? 
            // Does anyone invent his own headers these days? 
            // Read the fucking RFC guys! This header is not there!
            cookieStr +=12;
            // Continue with lastCookie
        }
        else
        {
            // This is not the start of a cookie header, skip till next line.
            while (*cookieStr && *cookieStr != '\n')
                cookieStr++;
            
            if (*cookieStr == '\n')
            	cookieStr++;
             
            if (!*cookieStr)
                break; // End of cookie_headers
            else
                continue; // end of this header, continue with next.
        }
        
        while ((*cookieStr == ';') || (*cookieStr == ' '))
        {
            cookieStr++;

            // Name-Value pair follows
            cookieStr = parseNameValue(cookieStr, Name, Value);
                
            Name = Name.lower();

            if (Name == "domain")
            {
                lastCookie->domain = Value.lower();
            }
            else if (Name == "max-age")
            {
                int max_age = Value.toInt();
                if (max_age == 0)
                    lastCookie->expireDate = 1;
                else
                    lastCookie->expireDate = time(0)+max_age;
            }
            else if (Name == "expires")
            {
                // Parse brain-dead netscape cookie-format
            }
            else if (Name == "path")
            {
                lastCookie->path = Value;
            }
            else if (Name == "version")
            {
                lastCookie->protocolVersion = Value.toInt();
            }
        }
        if (*cookieStr == '\0')
            break; // End of header

        // Skip ';' or '\n'
        cookieStr++;
    }
    
    return cookieChain;
}

//
// This function hands a KCookie object over to the cookie jar.
//
// On return cookiePtr is set to 0.
//
void KCookieJar::addCookie(KCookiePtr &cookiePtr)
{
    QString domain;
    stripDomain( cookiePtr->host, domain);
    KCookieList *cookieList = cookieDomains[domain.data()];
    
    if (!cookieList)
    {
        // Make a cookie list for domain 

        // Make a new cookie list
        cookieList = new KCookieList();
        cookieList->setAdvice( KCookieDunno );
            
        cookieDomains.insert( domain.data(), cookieList);

        // Update the list of domains
        domainList.append( domain.data());
    }

    // Look for matching existing cookies
    // They are removed
    for ( KCookiePtr cookie=cookieList->first(); cookie != 0; )
    {
        if ((cookie->name == cookiePtr->name) && 
            (cookie->domain == cookiePtr->domain) &&
            (cookie->path == cookiePtr->path))
        {
            KCookiePtr old_cookie = cookie;
            cookie = cookieList->next(); 
            cookieList->removeRef( old_cookie );
        }
        else
        {
            cookie = cookieList->next();
        }
    }


    // Add the cookie to the cookie list
    // The cookie list is sorted 'longest path first'
    if (!cookiePtr->isExpired(time(0)))
    {
        cookieList->inSort( cookiePtr );
    }
    else
    {
        delete cookiePtr;
    }
    cookiePtr = 0;
}

//
// This function advices whether a single KCookie object should 
// be added to the cookie jar.
//
KCookieAdvice KCookieJar::cookieAdvice(KCookiePtr cookiePtr)
{
    QString domain;
    stripDomain( cookiePtr->host, domain);
    // First check if the domain matches the host
    if (!cookiePtr->domain.isEmpty() &&
        (cookiePtr->domain != domain) && 
        (cookiePtr->domain != cookiePtr->host))
    {
        printf("WARNING: Host %s tries to set cookie for domain %s\n",
              cookiePtr->host.data(), cookiePtr->domain.data());
        return KCookieReject;
    }

    KCookieList *cookieList = cookieDomains[domain.data()];
    KCookieAdvice advice;
    
    if (cookieList)
    {
        advice = cookieList->getAdvice();
    }
    else
    {
        advice = globalAdvice;
    }

    return advice;    
}

//
// This function sets the advice for all cookies originating from 
// _domain.    
//
void KCookieJar::setDomainAdvice(const char *_domain, KCookieAdvice _advice)
{
    QString domain(_domain);
    KCookieList *cookieList = cookieDomains[domain.data()];
    
    if (cookieList)
    {
        // domain is already known
        cookieList->setAdvice( _advice);

        if ((cookieList->isEmpty()) && 
            (_advice == KCookieDunno))
        {
            // This deletes cookieList!
            cookieDomains.remove(domain.data());    
            
            domainList.remove(domain.data());            
        }
    }
    else
    {
        // domain is not yet known
        if (_advice != KCookieDunno)
        {
            // We should create a domain entry
            
            // Make a new cookie list
            cookieList = new KCookieList();
            cookieList->setAdvice( _advice);
            
            cookieDomains.insert( domain.data(), cookieList);

            // Update the list of domains
            domainList.append( domain.data());
        }
    }
        
}

//
// This function sets the global advice for cookies 
//
void KCookieJar::setGlobalAdvice(KCookieAdvice _advice)
{
    globalAdvice = _advice;
}

//
// Get a list of all domains known to the cookie jar.
//
const QStrList *KCookieJar::getDomainList()
{
    return &domainList;
}
    
//
// Get a list of all cookies in the cookie jar originating from _domain.
//
const KCookieList *KCookieJar::getCookieList(const char *_domain)
{
    return cookieDomains[_domain];
}

//
// Eat a cookie out of the jar. 
// cookiePtr should be one of the cookies returned by getCookieList()
//
void KCookieJar::eatCookie(KCookiePtr cookiePtr)
{
    QString domain;
    stripDomain( cookiePtr->host, domain);
    KCookieList *cookieList = cookieDomains[domain.data()];
    
    if (cookieList)
    {
        // This deletes cookiePtr!
        cookieList->removeRef( cookiePtr );

        if ((cookieList->isEmpty()) && 
            (cookieList->getAdvice() == KCookieDunno))
        {
            // This deletes cookieList!
            cookieDomains.remove(domain.data());    
            
            domainList.remove(domain.data());            
        }
    }
}    

