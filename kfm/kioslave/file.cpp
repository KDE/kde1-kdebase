#include "file.h"
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

KProtocolFILE::KProtocolFILE()
{
	file = NULL;
	dp = 0L;
	allowHTML = FALSE;
}

KProtocolFILE::~KProtocolFILE()
{
	Close();
}

int KProtocolFILE::Open(KURL *url, int mode)
{
    QString p( url->path() );
    KURL::decodeURL( p );
    
	struct stat st;
	Close();

	size=0;

	if(mode & READ)
	{

		file = fopen( p, "rb");
		if(!file)
		{
			Error(KIO_ERROR_CouldNotRead,"Can't open File for reading",errno);
			return(FAIL);
		}
		fstat(fileno(file), &st);
		size = st.st_size;
	}
	if(mode & WRITE)
	{
		if(!(mode & OVERWRITE))
		{
			if(stat( p, &st) != -1)
			{
			    Error(KIO_ERROR_FileExists,"File already exists",errno);
				return(FAIL);
			}
		}

		file = fopen( p, "wb");
		if(!file)
		{
			Error(KIO_ERROR_CouldNotRead,"Can't open file for reading",errno);
			return(FAIL);
		}
	}
	return SUCCESS;
}

int KProtocolFILE::Close()
{
	if(file) fclose(file);
	file = NULL;
	return SUCCESS;
}

long KProtocolFILE::Read(void *buffer, long len)
{
	long n = fread(buffer,1,len,file);
	if(n < 0)
	{
		Error(KIO_ERROR_CouldNotRead,"Reading from file failed",errno);
		return(FAIL);
	}
	return(n);
}

long KProtocolFILE::Write(void *buffer, long len)
{
	long n = fwrite(buffer,1,len,file);
	if(n < 0)
	{
		Error(KIO_ERROR_CouldNotRead,"Writing to file failed",errno);
		return(FAIL);
	}
	return(n);
}

long KProtocolFILE::Size()
{
	if(!file) return(FAIL);
	return(size);
}

int KProtocolFILE::atEOF()
{
	if(!file) return(FAIL);
	return(feof(file));
}

int KProtocolFILE::OpenDir( KURL *url )
{
    file = 0L;
    dp = 0L;
    
    // Open the directory
    path = url->path();
    KURL::decodeURL(path);
    // Save a copy
    QString tmp = path.data();
    if ( path.right(1) != "/" )
	path += "/";

    // Are we allows to respond with HTML ?
    if ( allowHTML )
    {
	// Try to open index.html
	QString t( path.data() );
	t += "index.html";
	FILE *f = fopen( t.data(), "rb" );
	if ( f )
	{
	    file = f;
	    return SUCCESS;
	}
	// Try to open .kde.html
	t = path.data();
	t += ".kde.html";
	f = fopen( t.data(), "rb" );
	if ( f )
	{
	    file = f;
	    return SUCCESS;
	}
    }
    
    dp = opendir( tmp );
    if ( dp == 0L )
	return FAIL;

    if ( path.right(1) != "/" )
	path += "/";
    
    return SUCCESS;
}

void KProtocolFILE::EmitData( KIOSlaveIPC *_ipc )
{
    // Read all data from 'file'
    char buffer[ 4100 ];
    while ( !feof( file ) )
    {
	int n = fread( buffer, 1, 4096, file );
	if ( n > 0 )
	{
	    buffer[n] = 0;
	    _ipc->data( buffer );
	}
    }
}

KProtocolDirEntry *KProtocolFILE::ReadDir()
{
    struct dirent *ep;
    // Loop thru all directory entries
    while ( ( ep = readdir( dp ) ) != 0L )
    {
    	QString name(ep->d_name);
	KURL::encodeURL(name);

	QString fname = path.data();
	fname += ep->d_name;

	struct stat buff;
	int stat_ret = stat( fname, &buff );
	struct stat lbuff;
	/* int lstat_ret = */ lstat( fname, &lbuff );
	struct tm *t = localtime( &lbuff.st_mtime );
	struct passwd * user = getpwuid( buff.st_uid );
	struct group * grp = getgrgid( buff.st_gid );

	char buffer[1024];
	static KProtocolDirEntry de;
	
	de.isdir = FALSE;
	if ( S_ISLNK( lbuff.st_mode ) )
	    buffer[0] = 'l';		
	else if ( S_ISDIR( buff.st_mode ) )
	{
	    buffer[0] = 'd';
	    de.isdir = TRUE;
	}
	else
	    buffer[0] = '-';
	
	char uxbit,gxbit,oxbit;
 
	if ( (buff.st_mode & (S_IXUSR|S_ISUID)) == (S_IXUSR|S_ISUID) )
	    uxbit = 's';
	else if ( (buff.st_mode & (S_IXUSR|S_ISUID)) == S_ISUID )
	    uxbit = 'S';
	else if ( (buff.st_mode & (S_IXUSR|S_ISUID)) == S_IXUSR )
	    uxbit = 'x';
	else
	    uxbit = '-';
	
	if ( (buff.st_mode & (S_IXGRP|S_ISGID)) == (S_IXGRP|S_ISGID) )
	    gxbit = 's';
	else if ( (buff.st_mode & (S_IXGRP|S_ISGID)) == S_ISGID )
	    gxbit = 'S';
	else if ( (buff.st_mode & (S_IXGRP|S_ISGID)) == S_IXGRP )
	    gxbit = 'x';
	else
	    gxbit = '-';
	
	if ( (buff.st_mode & (S_IXOTH|S_ISVTX)) == (S_IXOTH|S_ISVTX) )
	    oxbit = 't';
	else if ( (buff.st_mode & (S_IXOTH|S_ISVTX)) == S_ISVTX )
	    oxbit = 'T';
	else if ( (buff.st_mode & (S_IXOTH|S_ISVTX)) == S_IXOTH )
	    oxbit = 'x';
	else
	    oxbit = '-';
    
	buffer[1] = ((( buff.st_mode & S_IRUSR ) == S_IRUSR ) ? 'r' : '-' );
	buffer[2] = ((( buff.st_mode & S_IWUSR ) == S_IWUSR ) ? 'w' : '-' );
	buffer[3] = uxbit;
	buffer[4] = ((( buff.st_mode & S_IRGRP ) == S_IRGRP ) ? 'r' : '-' );
	buffer[5] = ((( buff.st_mode & S_IWGRP ) == S_IWGRP ) ? 'w' : '-' );
	buffer[6] = gxbit;
	buffer[7] = ((( buff.st_mode & S_IROTH ) == S_IROTH ) ? 'r' : '-' );
	buffer[8] = ((( buff.st_mode & S_IWOTH ) == S_IWOTH ) ? 'w' : '-' );
	buffer[9] = oxbit;
	buffer[10] = 0;

	// A link pointing to nowhere ?
	if ( stat_ret == -1 && S_ISLNK( lbuff.st_mode ) )
	    strcpy( buffer, "lrwxrwxrwx" );
	
	de.access = buffer;
	de.name = name.data();
	de.owner = (( user != 0L ) ? user->pw_name : "???" );
	de.group = (( grp != 0L ) ? grp->gr_name : "???" );
	QString d;
	d.sprintf("%02i:%02i %02i.%02i.%02i", t->tm_hour,t->tm_min,t->tm_mday,t->tm_mon + 1,t->tm_year );
	de.date = d.data();
	de.size = buff.st_size;
	if ( de.isdir )
	    de.name += "/";

	return &de;
    }
    return NULL;
}

int KProtocolFILE::CloseDir()
{
    if ( file )
	fclose( file );
    if ( dp )
	closedir( dp );
    dp = 0L;
    return SUCCESS;
}

int KProtocolFILE::MkDir(KURL *url)
{
    QString p( url->path() );
    KURL::decodeURL( p );
    
    if ( ::mkdir( p, S_IXUSR | S_IWUSR | S_IRUSR ) != 0L )
	return Error( KIO_ERROR_CouldNotMkdir, "Could not mkdir '%s'", errno );
    return SUCCESS;
}

int KProtocolFILE::GetPermissions( KURL &_u )
{
    QString p( _u.path() );
    KURL::decodeURL( p );
    
    struct stat buff;
    stat( p, &buff );

    printf("!!!!!!! Returning Permissions '%i'\n", (int)(buff.st_mode));

    return (int)(buff.st_mode);
}

void KProtocolFILE::SetPermissions( KURL &_u, int _perms )
{
    if ( _perms == -1 )
	return;
    
    printf("!!!!!!!! Setting Permissions '%i'\n", _perms);
    chmod( _u.path(), (mode_t)_perms );                     
}

#include "file.moc"
