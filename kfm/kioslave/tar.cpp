#include "tar.h"
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>

KProtocolTAR::KProtocolTAR()
{
    len = 0;
    hasdata = TRUE;
    bEOF = FALSE;
}

KProtocolTAR::~KProtocolTAR()
{
}

int KProtocolTAR::AttachTAR( const char *_command )
{
    //debug("KProtocolTAR::AttachTAR(command=%s)",_command);
    InitParent();
    
    KURL uparent( ParentURL );
    if( Parent->Open( &uparent, READ ) == FAIL )
	return FAIL;

    char buffer[ 3 ];
    int len = Parent->Read( buffer, 2 );
    if ( len != 2 )
	return FAIL;
    
    QString cmd;
    // GZIP file ?
    if ( (unsigned char)buffer[0] == 0x1f && (unsigned char)buffer[1] == 0x8b )
	cmd.sprintf( _command, "z" );
    else
	cmd.sprintf( _command, "" );

    if( Slave.Start( cmd ) == FAIL )
	return FAIL;
    write( Slave.in, buffer, 2 );
    
    Slave.SetNDelay(KSlave::IN | KSlave::OUT | KSlave::ERR);
    return SUCCESS;
}

int KProtocolTAR::Open( KURL *url, int mode )
{
    //debug("KProtocolTAR::Open(url=%s)",url->url().data());
    if( mode & READ )
    {
	const char *path = url->path();

	// extracting /xxx from a tarfile containing the file xxx won't work
	while( *path == '/' ) path++;	

	QString Command( "tar -%sOxf - \"" );
	Command += path;
	Command += "\"";
	return( AttachTAR( Command ) );
    }
    return FAIL;
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
 
    // Read until eof or until the '_buffer' is filled up
    do
    {
	int iomask = Slave.WaitIO( 1, 0 );

	// Does the slave has something to read for us ?
	if ( iomask & KSlave::OUT )
	{
	    count = read( Slave.out, (char*)_buffer + pos, _len - pos );
	    if( count > 0 )
		pos += count;
	    if( count == -1 )
		perror("KProtocolTAR: read didn't work");
	}
	// Does the slave ask for new input ?
	if ( iomask & KSlave::IN || iomask == 0 )
	    HandleRefill();

    } while( ( pos != _len ) && ( count != 0 ) );

    if ( count <= 0 )
    {
	bEOF = TRUE;
    }
    return(pos);
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
    return 0x7fffffff;		// dunno ... ;)
}

int KProtocolTAR::OpenDir(KURL *url)
{
    subdir = "";
    dirpathmem = dirpath = strdup( url->path() );

    // extracting /xxx from a tarfile containing the file xxx won't work
    while( dirpath[0] == '/' ) dirpath++;	
    
    QString Command( "tar -%stvf -");
    int rc = AttachTAR( Command.data() );
    dirfile = fdopen( Slave.out, "r" );
    return rc;
}

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
				return( &de );
			    }
			}
		    }
		}
	    // when URL doesn't pass QC, give it to cachemanager (to be written)
	}
    } while( readstr );
    return NULL;
}

int KProtocolTAR::CloseDir()
{
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

#include "tar.moc"
