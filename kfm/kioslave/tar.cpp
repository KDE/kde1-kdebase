#include "tar.h"
#include "kio_errors.h"
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

KProtocolTAR::KProtocolTAR()
{
    len = 0;
    hasdata = TRUE;
    bEOF = FALSE;
    dirlist.setAutoDelete(true);
}

KProtocolTAR::~KProtocolTAR()
{
    // debug("KProtocolTAR::~KProtocolTAR()");
    dirlist.clear();
}

int KProtocolTAR::isGzip()
{
    InitParent();
    KURL uparent( ParentURL );
    if( Parent->Open( &uparent, READ ) == FAIL )
	return Error(KIO_ERROR_CouldNotRead, "could not read",0) ;

    char buffer[ 3 ];
    int len = Parent->Read( buffer, 2 );
    if ( len != 2 )
	return Error(KIO_ERROR_CouldNotRead, "could not read",0) ;
    
    // GZIP file ?
    int isgz = ( (unsigned char)buffer[0] == 0x1f && 
             (unsigned char)buffer[1] == 0x8b );    
    Parent->Close();
    return isgz;
}

int KProtocolTAR::AttachTAR( const char *_command )
{
    // debug("KProtocolTAR::AttachTAR(command=%s)",_command);
    InitParent();

    int bGzip = isGzip();
    if (bGzip == FAIL) return FAIL;

    KURL uparent( ParentURL );
    if( Parent->Open( &uparent, READ ) == FAIL )
	return Error(KIO_ERROR_CouldNotRead, "could not read",0) ;

    QString cmd;
    // GZIP file ?
    if ( bGzip )
	cmd.sprintf( _command, "z" );
    else
	cmd.sprintf( _command, "" );
    if( Slave.Start( cmd ) == FAIL )
        return Error(KIO_ERROR_CouldNotRead, "could not read",0) ;
    
    Slave.SetNDelay(KSlave::IN | KSlave::OUT | KSlave::ERR);
    return SUCCESS;
}

int KProtocolTAR::Open( KURL *url, int mode )
{
    // debug("KProtocolTAR::Open(url=%s)",url->url().data());
    if( mode & READ )
    {
	const char *path = url->path();

	// extracting /xxx from a tarfile containing the file xxx won't work
	while( *path == '/' ) path++;	

	QString Command( "tar -%sOxf - \"" );
	Command += path;
	Command += "\"";
	dirfile = fdopen( Slave.out, "r" );
	int rc = ( AttachTAR( Command ) );
	//dirfile = fdopen( Slave.out, "r" );
	return rc;
    }
    return Error( KIO_ERROR_NotImplemented, "can't write to a tar file", 0);
}

bool KProtocolTAR::HandleRefill()
{
    // Did we write everything we have and does the parent
    // protocol provide more data ?
    if( !Parent->atEOF() && !hasdata )
    {
	// Read from the parent
	len = Parent->Read( internalbuffer, 1024 );
	if( len > 0 )
	    hasdata = TRUE;
    }
    if( hasdata )
    {
	// Write our stuff to the tar process
	if( write( Slave.in, internalbuffer, len ) != -1 )
	    // No data stored yet
	    hasdata = FALSE;
	return true;
    }

    if ( Slave.isClosed() )
	return 0;

    Slave.Close();
    Slave.SetNDelay( 0 );
    return false;
}

long KProtocolTAR::Read( void *_buffer, long _len )
{
    long pos = 0;
    ssize_t count = -1;
    int moredata = 0;
 
    // Read until eof or until the '_buffer' is filled up
    do
    {
      int iomask = Slave.WaitIO( 1, 0 );
      if( iomask & KSlave::IN || iomask == 0 )
	moredata = HandleRefill();
      if( iomask & KSlave::OUT || !moredata )
      {
	count = read (Slave.out, (char*)_buffer + pos, _len-pos);
	//debug ("read %d bytes", count);
	if (count == -1)
	{
	  if (errno == EAGAIN)
	    continue;           //Not ready
	  else
	    return -1;          // Error;
	}
	
	if (count == 0)
	{
	  //debug ("EOF");
	  bEOF = true;
	  return pos;
	}
	pos += count;
      }
    }
    while( pos != _len );
    return pos;
}

int KProtocolTAR::Close()
{
  Slave.Stop();
  return Parent->Close();
}

int KProtocolTAR::atEOF()
{
    return bEOF;
}

long KProtocolTAR::Size()
{
    /* This is a bit hard to implement.
       Opendir/Readdir get the size, but the class is destroyed between
       the 'list' and the 'copy' commands.
       Anyway, it's just for the progress bar, which is very fast for local 
       tar files ! */
    return FAIL;		// dunno ... ;)
}

bool KProtocolTAR::isDirStored( const char *_name )
{
    if (dirlist.count()==0)  // empty list ?
        return FALSE;
    QString s(_name);
    s += "/"; // trailing slash added (this method is for dirs only)
    for ( KProtocolDirEntry *de = dirlist.first(); de != 0L; de = dirlist.next() )
    {
        if (s == de->name)
            return TRUE;
    }
    return FALSE;
}

// Returns tmp made relative to dirpath
int KProtocolTAR::makeRelative(QString & tmp)
{
    if (!dirpath.isEmpty())               // dirpath not empty?
    {  
        if (tmp.find(dirpath) == 0)         // no, find dir we want
            tmp.remove(0, dirpath.length());   // and remove it.
        else {
            // this happens when trying manually to add a trailing '/' to a 
            // file name in the tar file. A bug in the tar program,
            // actually. David.
            debug("ERROR ! dirpath=%s not contained in tmp=%s",dirpath.data(),tmp.data());
            return FAIL; // Error to be set by caller.
        }
    }
    
    if (tmp[0] == '/')                // starts with slash ?
        tmp.remove (0, 1);              // Yeah remove it
    return TRUE;
}

int KProtocolTAR::OpenDir(KURL *url)
{
    dirpath = url->path();

    // extracting /xxx from a tarfile containing the file xxx won't work
    while( dirpath[0] == '/' ) dirpath.remove(0,1);
    
    QString Command( "tar -%stvf -");
    if (dirpath[0] != '\0')
    {
      Command += " \"";
      Command += dirpath;
      Command += "\"";
    }
    int rc = AttachTAR( Command.data() );
    if (rc == FAIL) return FAIL;
    dirfile = fdopen( Slave.out, "r" );
    // debug ("Tar: opendir for %s", dirpath.data());
    dirlist.clear();

    char buffer[1024];
    char *readstr = "ok";		// to prevent breaking the loop at startup
    int moredata = 1;
    KProtocolDirEntry *de;

    do
    {
      int iomask = Slave.WaitIO( 1, 0 );
      if( iomask & KSlave::IN || iomask == 0 )
	    moredata = HandleRefill();
      if( iomask & KSlave::OUT || !moredata )
      {
	char p_access[80], p_owner[80], p_group[80];
	char p_size[80], p_date[80], p_name[250];

	readstr = fgets(buffer,1024,dirfile);

	if (readstr && (sscanf(buffer,
   " %[-dlrwxst] %[0-9.a-zA-Z_]/%[0-9.a-zA-Z_] %[0-9] %17[a-zA-Z0-9:- ] %[^\n]",
   // is \r necessary, together with \n ? Not here...
			       p_access, p_owner, p_group,
			       p_size, p_date, p_name) == 6))
	{
	  // Link?
	  QString tmp( p_name );
	  if ( p_access[0] == 'l' )
	  {
	    int i = tmp.findRev( " -> " );
	    if ( i != -1 )
	      tmp.truncate( i ); //forget link
	  }
          
          if (makeRelative(tmp)==FAIL)
              return Error(KIO_ERROR_CouldNotList, "No such dir", 0);

	  int i = tmp.find ("/");
	  // this is first slash; If there is something behind this slash,
	  // these are files in that subdir and we don't want to see them.

	  if (i != -1)
	  {
	    if (tmp[i+1] != '\0') // yes there is something
            {
              // debug("there is something : %s, tmp1=%s",tmp.data(),tmp1.data());
              // hack to add missing parent directories if necessary.
	      tmp.truncate(i);
	      if (isDirStored(tmp.data()))
                  continue;           // Ignore it
	      debug("emulating dir %s",tmp.data());
              // otherwise, create wrong information about a directory and 
              // add it to dirlist, which is done below.
              strcpy(p_access, "d?????????");
              strcpy(p_size, "0");
              strcpy(p_owner, "???");
              strcpy(p_group, "???");
            }
	    else
	      tmp.truncate(i);
	  }
	  // else? this is a file;

	  // now when we removed the path and slashes, If this an
	  // empty string, then it can be either dir name itself
	  // or a file with that name
	  if (tmp.isEmpty())
	  {
	    if (p_access[0] != 'd')
	    {
	      debug ("bad, you tried to openDir a file");
	      CloseDir();
	      return FAIL;
	    }
	  }
	  else
	  {
	    de = new KProtocolDirEntry;

	    de->access  = p_access;
	    de->owner	= p_owner;
	    de->group	= p_group;
	    de->size	= atoi(p_size);
	    de->isdir	= ( p_access[0] == 'd' );
	    de->name	= tmp.data();
	    if( de->isdir )
	      de->name += "/";
	    de->date = p_date;
	    dirlist.append(de);
	    // debug ("Tar: openDir: %s", de->name.data());
	  }
	} else if (readstr) { fprintf(stderr,"!! Couldn't parse %s",buffer); }
      }
    }
    while( readstr );
    if (dirlist.count()==0) // tar returned nothing : probably a wrong dir
        return Error(KIO_ERROR_CouldNotList, "No such dir", 0);
    dirlist.first();
    return SUCCESS;
}

int KProtocolTAR::CloseDir()
{
  //debug ("Tar: dir closed %s",  dirpath.data());
  if( dirfile )
  {
    fclose( dirfile );
    dirfile = 0L;
    Parent->Close();
    Slave.Stop();
  }
  return SUCCESS;
}

KProtocolDirEntry *KProtocolTAR::ReadDir()
{
  KProtocolDirEntry *de = dirlist.current();
//  if (de)
  //  debug ("Tar: ReadDir: %s", de->name.data());
//  else
  //  debug ("Tar: ReadDir: NULL");
  dirlist.next();
  return de;
}

/*  David tried but didn't succeed. Compressed archives don't support
    updates, and anyway the problem is that it's needed to create the 
    whole hierarchy first, before being able to tar -rf ...
    
int KProtocolTAR::MkDir(KURL * url)
{
    debug("Tar: Mkdir: %s",url->path());

    int bGzip = isGzip();
    if (bGzip == FAIL) return FAIL;

    const char *path = url->path();

    // extracting /xxx from a tarfile containing the file xxx won't work
    while( *path == '/' ) path++;	
        
    QString cmdTempl( "tar -%srf %s \"%s\"" );
    QString cmd;
    QString parentURL = ParentURL.remove(0,5); // skip "file:"

    if ( bGzip ) {
        // cmd.sprintf( cmdTempl, "z", parentURL.data(), path );
        return Error(KIO_ERROR_NotPossible, "Cannot update compressed archives", 0);
    }
    else
        cmd.sprintf( cmdTempl, "", parentURL.data(), path );

    debug(cmd);
    return SUCCESS;
}*/

/* TODO:
int KProtocolTAR::Delete(KURL *)
{

}
*/

/* Sven commented out without touching
 
KProtocolDirEntry *KProtocolTAR::ReadDir()
{
    char buffer[1024];
    static KProtocolDirEntry de;
    int moredata = 1;
    char *readstr = "ok";		// to prevent breaking the loop at startup
    
    do 
    {
	int iomask = Slave.WaitIO( 1, 0 );
	if( iomask & KSlave::IN || iomask == 0 )
	    moredata = HandleRefill();
	if( iomask & KSlave::OUT || !moredata )
	{
            char p_access[80], p_owner[80], p_group[80];
            char p_size[80], p_date[80], p_name[250];
	    
	    readstr = fgets(buffer,1024,dirfile);

            // strtok calls removed. sscanf allows to deal with GNU tar and
            // non-GNU tar :
            // GNU tar returns the date in the form 1998-12-27 22:13
            // non-GNU tar returns it   in the form Nov 19 19:33 1997
            if (readstr && (sscanf(buffer,
             " %[-drwxst] %[0-9.a-zA-Z_]/%[0-9.a-zA-Z_] %[0-9] %17[a-zA-Z0-9:- ] %[^\n]",
                                  // is \r necessary, together with \n ? Not here...
                        p_access, p_owner, p_group, p_size, p_date, p_name) == 6) &&
		( !strlen( dirpath ) || strncmp( p_name, dirpath, strlen( dirpath ) ) == 0 ) )
		{
		    if( p_name[ strlen( p_name ) - 1 ] == '/' )
			p_name[ strlen( p_name ) - 1 ] = 0;
		    if ( p_access[0] == 'l' )
		    {
			QString tmp( p_name );
			int i = tmp.findRev( " -> " );
			if ( i != -1 )
			    tmp.truncate( i );
			strcpy( p_name, tmp.data() );
		    }
		    
		    if ( strlen( dirpath ) < strlen( p_name ) )
		    {
                        // make p_relname (relative file name) point to
                        // the relative file path within p_name
			char * p_relname = p_name + strlen( dirpath );
			if( !strchr( p_relname, '/' ) )
			{
			    subdir = p_relname;
			    de.access	= p_access;
			    de.owner	= p_owner;
			    de.group	= p_group;
			    de.size	= atoi(p_size);
			    de.isdir	= ( p_access[0] == 'd' );
			    de.name	= p_relname;
			    if( de.isdir )
				de.name += "/";
			    de.date = p_date;
			    debug ("Tar: readDir: %s", de.name.data());
			    return( &de );
			}
			else if ( p_access[0] == 'd' )
			{
			    char *p = strchr( p_relname, '/' );
			    *p = 0;
			    if ( subdir != p_relname )
			    {
				subdir = p_relname;
				de.access	= p_access;
				de.owner	= p_owner;
				de.group	= p_group;
				de.size	        = 0;
				de.isdir	= true;
				de.name	        = p_relname;
				de.name += "/";
				de.date = p_date;
				debug ("Tar: readDir: %s", de.name.data());
				return( &de );
			    }
			}
		    }
		}
	    // when URL doesn't pass QC, give it to cachemanager (to be written)
	}
    } while( readstr );
    debug ("Tar: readDir: NULL");
    return NULL;
}


int KProtocolTAR::CloseDir()
{
  debug ("Tar: dir closed %s",  dirpathmem);
    if( dirfile )
    {
	free( dirpathmem );
	fclose( dirfile );
	dirfile = NULL;
	Parent->Close();
	Slave.Stop();
    }
    return SUCCESS;
}

*/
#include "tar.moc"
