#include "tar.h"
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

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
            char *p_access = 0L, *p_owner = 0L, *p_group = 0L;
            char *p_size = 0L, *p_date_1 = 0L, *p_date_2 = 0L, *p_name = 0L;
            // char *p_date_3 = 0L, *p_date_4 = 0L;
	    
	    readstr = fgets(buffer,1024,dirfile);
	    
	    if( readstr && (p_access = strtok(buffer," ")) != 0 && (p_owner = strtok(NULL,"/")) != 0 &&
		(p_group = strtok(NULL," ")) != 0 && (p_size = strtok(NULL," ")) != 0 &&
		(p_date_1 = strtok(NULL," ")) != 0 && (p_date_2 = strtok(NULL," ")) != 0 &&
//		(p_date_3 = strtok(NULL," ")) != 0 && (p_date_4 = strtok(NULL," ")) != 0 &&
// removed by David. Looks like tar output has changed. Here it looks like :
// -rw-r--r-- user/group  2858 1998-12-27 22:13 dir/file
// so there are only two items for the date & time, not 4.
// This is with GNU tar 1.12. Which tar has 4 items ? Old GNU tar or 
// non-GNU tars ? Testing the tar version might be necessary...
		(p_name = strtok(NULL,"\r\n")) != 0 &&
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
			p_name += strlen( dirpath );
			if( !strchr( p_name, '/' ) )
			{
			    subdir = p_name;
			    de.access	= p_access;
			    de.owner	= p_owner;
			    de.group	= p_group;
			    de.size	= atoi(p_size);
			    de.isdir	= ( p_access[0] == 'd' );
			    de.name	= p_name;
			    if( de.isdir )
				de.name += "/";
			    //de.date.sprintf("%s %s %s",p_date_1,p_date_2,p_date_4);
                            de.date = p_date_1;
			    /* doesn't understand time */
			    return( &de );
			}
			else if ( p_access[0] == 'd' );
			{
			    char *p = strchr( p_name, '/' );
			    *p = 0;
			    if ( subdir != p_name )
			    {
				subdir = p_name;
				de.access	= p_access;
				de.owner	= p_owner;
				de.group	= p_group;
				de.size	        = 0;
				de.isdir	= true;
				de.name	        = p_name;
				de.name += "/";
				//de.date.sprintf("%s %s %s",p_date_1,p_date_2,p_date_4);
                                de.date = p_date_1;
				/* doesn't understand time */
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
