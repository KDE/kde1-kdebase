#include "gzip.h"
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include "kio_errors.h"

KProtocolGZIP::KProtocolGZIP()
{
    hasdata = FALSE;
    len = 0;
    bEOF = FALSE;
}

KProtocolGZIP::~KProtocolGZIP()
{
}

int KProtocolGZIP::OpenGZIP()
{
    InitParent();
    
    KURL uparent( ParentURL );
    if( Parent->Open( &uparent, READ ) == FAIL )
	return FAIL;

    QString Command;
    Command.sprintf( "gzip -dc" );
    if( Slave.Start( Command.data() ) == FAIL )
	return FAIL;
    
    Slave.SetNDelay(KSlave::IN | KSlave::OUT | KSlave::ERR);
 
    return SUCCESS;
}

int KProtocolGZIP::Open(KURL *, int mode)
{
    // We dont care about the URL, since gzip is just a filter
    if( mode & READ )
    {
	return( OpenGZIP() );
    }
    return(FAIL);
}

bool KProtocolGZIP::HandleRefill()
{
    if( !Parent->atEOF() && hasdata == 0 )
    {
	len = Parent->Read( internalbuffer, 1024 );
	if( len>0 )
	    hasdata = TRUE;
    }
    if( hasdata )
    {
	if( write( Slave.in, internalbuffer, len ) != -1 )
	    hasdata = FALSE;
	return TRUE;
    }

    if ( Slave.isClosed() )
	return 0;
    Slave.Close();
    Slave.SetNDelay(0);
    return FALSE;
}

long KProtocolGZIP::Read(void *buffer, long len)
{
    long pos = 0,count=-1;
    bool moredata = TRUE;
    
    do
    {
	int iomask = Slave.WaitIO(1,0);

	if( moredata && ( ( iomask & KSlave::IN ) || iomask == 0 ) )
	    moredata = HandleRefill();
	if( iomask & KSlave::OUT )
	{
	    count = read( Slave.out, (char*)buffer + pos, len - pos );
	    if( count > 0 )
		pos += count;
	    if( count == -1 )
		perror("KProtocolGZIP: read didn't work");
	}

    } while( ( pos != len ) && ( count != 0 ) );

    if( count <= 0 )
    {
	bEOF = 1;
    }
    return pos;
}

int KProtocolGZIP::Close()
{
    Slave.Stop();
    return(Parent->Close());
}

int KProtocolGZIP::atEOF()
{
    return bEOF;
}

long KProtocolGZIP::Size()
{
    return 0x7fffffff;		// dunno ... ;)
}

int KProtocolGZIP::OpenDir( KURL * )
{
    // This is not really an error. The protocol does can not support it
    // since the operation is technical impossible
    return ( Error( KIO_ERROR_NotPossible, "This is not possible...") ); 
}

#include "gzip.moc"
