#include "file.h"
#include <sys/stat.h>
#include <unistd.h>
#include <config-kfm.h>

KProtocolFILE::KProtocolFILE()
{
	file = NULL;
}

KProtocolFILE::~KProtocolFILE()
{
	Close();
}

int KProtocolFILE::Open(KURL *url, int mode)
{
	struct stat st;
	Close();

	size=0;

	if(mode & READ)
	{

		file = fopen(url->path(),"rb");
		if(!file)
		{
			Error(KIO_ERROR_CouldNotWrite,"Can't open File for writing",errno);
			return(FAIL);
		}
		fstat(fileno(file), &st);
		size = st.st_size;
	}
	if(mode & WRITE)
	{
		if(!(mode & OVERWRITE))
		{
			if(stat(url->path(), &st) != -1)
			{
			    Error(KIO_ERROR_FileExists,"File already exists",errno);
				return(FAIL);
			}
		}

		file = fopen(url->path(),"wb");
		if(!file)
		{
			Error(KIO_ERROR_CouldNotRead,"Can't open file for reading",errno);
			return(FAIL);
		}
	}
	return(SUCCESS);
}

int KProtocolFILE::Close()
{
	if(file) fclose(file);
	file = NULL;
	return SUCCESS;
}

int KProtocolFILE::Read(void *buffer, int len)
{
	long n = fread(buffer,1,len,file);
	if(n < 0)
	{
		Error(KIO_ERROR_CouldNotRead,"Reading from file failed",errno);
		return(FAIL);
	}
	return(n);
}

int KProtocolFILE::Write(void *buffer, int len)
{
	long n = fwrite(buffer,1,len,file);
	if(n < 0)
	{
		Error(KIO_ERROR_CouldNotWrite,"Writing to file failed",errno);
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

int KProtocolFILE::MkDir(KURL *url)
{
	if ( ::mkdir( url->path(), S_IXUSR | S_IWUSR | S_IRUSR ) != 0L )
	    return Error( KIO_ERROR_CouldNotMkdir, "Could not mkdir '%s'", errno );
	return SUCCESS;
}

int KProtocolFILE::GetPermissions( KURL &_u )
{
    struct stat buff;
    stat( _u.path(), &buff );

    debugT("!!!!!!! Returning Permissions '%i'\n", (int)(buff.st_mode));
    
    return (int)(buff.st_mode);
}

void KProtocolFILE::SetPermissions( KURL &_u, int _perms )
{
    if ( _perms == -1 )
	return;

    debugT("!!!!!!!! Setting Permissions '%i'\n", _perms);
    chmod( _u.path(), (mode_t)_perms );
}
