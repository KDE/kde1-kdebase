#include "file.h"
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>

int SortedKProtocolDirEntry::compareItems( GCI i1, GCI i2)
{
    KProtocolDirEntry *de1 = (KProtocolDirEntry *)i1;
    KProtocolDirEntry *de2 = (KProtocolDirEntry *)i2;
    if (sortMode & DIRSORT_DIRSFIRST)
    {
	if (de1->isdir != de2->isdir)
	    return ((de1->isdir) ? -1 : 1);
    }
    switch (sortMode & 0x7f)
    {
	case DIRSORT_BYNAME:
	    return strcmp( de1->name.data(), de2->name.data() );
	case DIRSORT_BYSIZE:
	    return de1->size - de2->size;
	default:
	    return strcmp( de1->name.data(), de2->name.data() );
    }
}

SortedKProtocolDirEntry::~SortedKProtocolDirEntry()
{
}
 

KProtocolFILE::KProtocolFILE()
{
	file = NULL;
	dlist = 0L;
	de = 0L;
	sortMode = (DIRSORT_DIRSFIRST | DIRSORT_BYNAME);
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
    DIR *dp = 0L;
    struct dirent *ep;
    struct stat buff;
    
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
	FILE *f;
	QString t( path.data() );
	t += "index.html";
	if (( stat( t.data(), &buff ) == 0 ) && S_ISREG( buff.st_mode ))
	    if (( f = fopen( t.data(), "rb" )))
	    {
		file = f;
		return SUCCESS;
	    }

	// Try to open .kde.html
	t = path.data();
	t += ".kde.html";
	if (( stat( t.data(), &buff ) == 0 ) && S_ISREG( buff.st_mode ))
	    if ((f = fopen( t.data(), "rb" )))
	    {
		file = f;
		return SUCCESS;
	    }
    }
    
    de = 0L;
    dp = opendir( tmp );
    if ( dp == 0L )
	return FAIL;

    if ( path.right(1) != "/" )
	path += "/";

    dlist = new SortedKProtocolDirEntry();
    dlist->sortMode = sortMode;

    while ( ( ep = readdir( dp ) ) != 0L )
    {
    	QString name(ep->d_name);
	KURL::encodeURL(name);

	QString fname = path.data();
	fname += ep->d_name;

	int stat_ret = stat( fname, &buff );
	struct stat lbuff;
	/* int lstat_ret = */ lstat( fname, &lbuff );
	struct tm *t = localtime( &lbuff.st_mtime );
	struct passwd * user = getpwuid( buff.st_uid );
	struct group * grp = getgrgid( buff.st_gid );

	char buffer[1024];
	KProtocolDirEntry *_de = new KProtocolDirEntry();
	
	_de->isdir = FALSE;
	if ( S_ISLNK( lbuff.st_mode ) )
	    buffer[0] = 'l';		
	else if ( S_ISDIR( buff.st_mode ) )
	{
	    buffer[0] = 'd';
	    _de->isdir = TRUE;
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
	
	_de->access = buffer;
	_de->name = name.data();
	_de->owner = (( user != 0L ) ? user->pw_name : "???" );
	_de->group = (( grp != 0L ) ? grp->gr_name : "???" );
	QString d;
	d.sprintf("%02i:%02i %02i.%02i.%02i", t->tm_hour,t->tm_min,t->tm_mday,t->tm_mon + 1,t->tm_year );
	_de->date = d.data();
	_de->size = buff.st_size;
	if ( _de->isdir )
	    _de->name += "/";

	if ( sortMode == DIRSORT_NONE )
	    dlist->append( _de );
	else
	    dlist->inSort( _de );
    }
    closedir( dp );
    de = dlist->first();

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
	    _ipc->data( IPCMemory( buffer, n ) );
	}
    }
}

KProtocolDirEntry *KProtocolFILE::ReadDir()
{
    // Loop thru all directory entries

    if (de) {
	KProtocolDirEntry *ode = de;
	de = dlist->next();
	return ode;
    }
    return NULL;
}

int KProtocolFILE::CloseDir()
{
    if ( file )
	fclose( file );
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
