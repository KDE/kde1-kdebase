#include "tar.h"
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

#define TAR_NORMAL  1
#define TAR_GZIP    2

int KProtocolTAR::AttachTAR(char *commandstr)
{
	unsigned char id[6];
	long pos = 0;
	int format = 0;

	InitParent();

	if(Parent->Open(&ParentURL,READ) == FAIL) return(FAIL);
	while(pos != 5)
	{
		long count = Parent->Read(&id[pos],5-pos);
		pos += count;
		if(count == FAIL) return(FAIL);
	}
	id[6]=0;

	if(strcmp((char *)id,"ustar") == 0) format = TAR_NORMAL;
	if(id[0] == 0x1f && id[1] == 0x8b) format = TAR_GZIP;

	if(!format) printf("can't determine format, filestart: %2x %2x %2x %2x\n",id[0],id[1],id[2],id[3]);
	if(!format) return(FAIL);

	printf("format is %s\n",format==TAR_GZIP?"gzipped tarfile":"uncompressed tarfile");

	QString Command;
	Command.sprintf(commandstr,format==TAR_GZIP?"z":"");

	printf("Starting tar: -> '%s' <-\n",Command.data());
	if(Slave.Start(Command.data()) == FAIL) return(FAIL);
	Slave.Write(id,5);

	Slave.SetNDelay(KSlave::IN | KSlave::OUT | KSlave::ERR);
	_EOF = 0;
	return(SUCCESS);
}

int KProtocolTAR::Open(KURL *url, int mode)
{
	if(mode & READ)
	{
		char *path = url->path();		// extracting /xxx from a tarfile
		while(path[0] == '/') path++;	// containing the file xxx won't work

		QString Command;
		Command.sprintf("tar -%%sOxf - %s",path);
		return(AttachTAR(Command.data()));
	}
	return(FAIL);
}

int KProtocolTAR::HandleRefill()
{
	static char internalbuffer[1024];
	static int hasdata = 0;
	static long len = 0;

	if(!Parent->atEOF() && hasdata == 0)
	{
		len = Parent->Read(internalbuffer,1024);
		//printf("HandleRefill: %ld Bytes read from Parent\n",len);
		if(len>0) hasdata = 1;
	}
	if(hasdata)
	{
		if(write(Slave.in,internalbuffer,len) != -1)
		{
			//printf("HandleRefill: written to tar(%ld bytes)\n",len);
			hasdata = 0;
		}
		return(1);
	}
	printf("Refill:slaveclose!\n");
	Slave.Close();
	Slave.SetNDelay(0);
	return(0);
}

long KProtocolTAR::Read(void *buffer, long len)
{
	long pos = 0,count=-1;

	printf("TAR< entering read\n");
	do
	{
		int iomask = Slave.WaitIO(1,0);

		if(iomask & KSlave::OUT)
		{
			count = read(Slave.out,buffer+pos,len-pos);
			if(count > 0) pos += count;
			if(count == -1) perror("KProtocolTAR: read didn't work");
			printf("pos now:%ld\n",pos);
		}
		if(iomask & KSlave::IN || iomask == 0) HandleRefill();
	} while((pos != len) && (count != 0));
	printf("TAR> ending read p:%ld,l:%ld,c:%ld\n",pos,len,count);
	if(count == 0)
	{
		printf("*** EOF ***\n");
		_EOF = 1;
	}
	return(pos);
}

int KProtocolTAR::Close()
{
	Slave.Stop();
	return(Parent->Close());
}

int KProtocolTAR::atEOF()
{
	return(_EOF);
}

long KProtocolTAR::Size()
{
	return(0x7fffffff);		// dunno ... ;)
}

int KProtocolTAR::OpenDir(KURL *url)
{
	dirpathmem = dirpath = strdup(url->path());
	while(dirpath[0] == '/') dirpath++;	

	// extracting /xxx from a tarfile
	// containing the file xxx won't work

	QString Command;
	Command.sprintf("tar -%%stvf -");
	int rc = AttachTAR(Command.data());
	dirfile = fdopen(Slave.out,"r");
	printf("requested dirpath is %s\n",dirpath);
	return(rc);
}

KProtocolDirEntry *KProtocolTAR::ReadDir()
{
	char buffer[1024];
	static KProtocolDirEntry de;
	int moredata = 1;
	char *readstr = "ok";		// to prevent breaking the loop at startup

	printf("tar ReadDir: called ; entering main loop\n");
	do
	{
		int iomask = Slave.WaitIO(1,0);

		if(iomask & KSlave::IN || iomask == 0)
			moredata = HandleRefill();
		if(iomask & KSlave::OUT || !moredata)
		{
			readstr = fgets(buffer,1024,dirfile);

			if(readstr)
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
						printf("PASSED QC... -> return\n");
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
	} while(readstr);
	printf("tar ReadDir: at eof!\n");
	return(NULL);
}

int KProtocolTAR::CloseDir()
{
	printf("--> CloseDir\n");
	if(dirfile)
	{
		free(dirpathmem);
		fclose(dirfile);
		dirfile = NULL;
		Parent->Close();
		Slave.Stop();
	}
	printf("<-- CloseDir\n");
	return(SUCCESS);
}

#include "tar.moc"
