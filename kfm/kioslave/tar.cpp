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
    printf("Starting tar: -> '%s' <-\n",cmd.data());
    if( Slave.Start( cmd ) == FAIL )
	return FAIL;
    write( Slave.in, buffer, 2 );
    
    Slave.SetNDelay(KSlave::IN | KSlave::OUT | KSlave::ERR);
    return SUCCESS;
}

int KProtocolTAR::Open( KURL *url, int mode )
{
    if( mode & READ )
    {
	char *path = url->path();		// extracting /xxx from a tarfile
	while( *path == '/' ) path++;	// containing the file xxx won't work
	QString Command;
	Command.sprintf( "tar -%%sOxf - %s", path );
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
    long pos = 0, count = -1;
    
    // Read untile eof or until the '_buffer' is filled up
    do
    {
	int iomask = Slave.WaitIO( 1, 0 );

	// Does the slave has something to read for us ?
	if ( iomask & KSlave::OUT )
	{
	    count = read( Slave.out, _buffer + pos, _len - pos );
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
    dirpathmem = dirpath = strdup( url->path() );
    while( dirpath[0] == '/' ) dirpath++;	
    
    // extracting /xxx from a tarfile
    // containing the file xxx won't work
    
    QString Command;
    Command.sprintf("tar -%%stvf -");
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
	    readstr = fgets(buffer,1024,dirfile);
	    if( readstr )
		if(char *p_access = strtok(buffer," "))
		    if(char *p_owner = strtok(NULL,"/"))
			if(char *p_group = strtok(NULL," "))
			    if(char *p_size = strtok(NULL," "))
				if(char *p_date_1 = strtok(NULL," "))
				    if(char *p_date_2 = strtok(NULL," "))
					if(char *p_date_3 = strtok(NULL," "))
					    if(char *p_date_4 = strtok(NULL," "))
						if(char *p_name = strtok(NULL," \r\n"))
						    if(!strlen(dirpath) || strncmp(p_name,dirpath,strlen(dirpath)) == 0)
						    {
							if(strlen(dirpath) < strlen(p_name))
							{
							    p_name += strlen(dirpath);
							    if(p_name[strlen(p_name)-1] == '/')
								p_name[strlen(p_name)-1]=0;
							    if(!strchr(p_name,'/'))
							    {
								de.access	= p_access;
								de.owner	= p_owner;
								de.group	= p_group;
								de.size		= atoi(p_size);
								de.isdir	= p_access[0]=='d';
								de.name		= p_name;
								if(de.isdir) de.name += "/";
								de.date.sprintf("%s %s %s",p_date_1,p_date_2,p_date_4);
								/* doesn't understand time */
								return(&de);
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
