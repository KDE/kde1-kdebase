#include "slave.h"
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <signal.h>

KSlave::KSlave()
{
    running = 0;
    in = -1;
    closed = true;
}

KSlave::~KSlave()
{
    Stop();
}

void KSlave::SetNDelay(int value)
{
    fcntl(in, F_SETFL,(value&IN) ?O_NDELAY:0);
    fcntl(out,F_SETFL,(value&OUT)?O_NDELAY:0);
    fcntl(err,F_SETFL,(value&ERR)?O_NDELAY:0);
}

int KSlave::WaitIO(long sec, long usec)
{
    timeval tv;
    fd_set rfds,wfds;
    int rc = 0;
    
    tv.tv_sec = sec;
    tv.tv_usec=usec;
    
    FD_ZERO(&rfds);
    FD_ZERO(&wfds);
    
    if(in != -1) FD_SET(in,&wfds);
    FD_SET(out,&rfds);
    FD_SET(err,&rfds);
    
    select( in + out + err + 1, &rfds, &wfds, NULL, &tv );
    
    if( in != -1 && FD_ISSET( in, &wfds ) )  rc |= IN;
    if( FD_ISSET( out, &rfds ) ) rc |= OUT;
    if( FD_ISSET( err, &rfds ) ) rc |= ERR;

    return rc;
}

int KSlave::Start( const char *command )
{
    closed = false;
    
    if( running ) return(0);
    
    if( !BuildPipe(&s_in,&in) ) return 0;
    if( !BuildPipe(&out,&s_out) ) return 0;
    if( !BuildPipe(&err,&s_err) ) return 0;

#ifdef SPECIAL_PIPE_TRICK
	fcntl(in, F_SETFL, O_NDELAY);
#endif

     SubProcess = fork();
     if( SubProcess == 0 )
     {
	 // init stdin, stdout and stderr for slave process
	 
	 fprintf(stderr,"KSlave: fork & pipes up, starting soon...!\n");
	 dup2(s_in,0);	fcntl(0,F_SETFD,0);
	 dup2(s_out,1);	fcntl(1,F_SETFD,0);
	 dup2(s_err,2);	fcntl(2,F_SETFD,0);
	 close(in);
	 close(out);
	 close(err);
	 close(s_in);
	 close(s_out);
	 close(s_err);
	 
	 char *argv[4] = { "/bin/sh","-c", NULL, NULL };
	 char *cmd = strdup( command );
	 argv[2] = cmd;
	 execv( argv[0], argv );
	 fprintf( stderr, "KSlave: exec failed...!\n" );
	 exit( 0 );
     }
     close(s_in);
     close(s_out);
     close(s_err);
     if( SubProcess == -1 )
	 return(0);
     running++;
     return 1;
}

int KSlave::BuildPipe(int *from, int *to)
{
    int pipe_fds[2];
    if( pipe( pipe_fds ) != -1 )
    {
	*from = pipe_fds[0];
	*to = pipe_fds[1];
	return 1;
    }
    return 0;
}

int KSlave::Close()
{
    if( in != -1 )
    {
	closed = true;
	close(in);
	in = -1;
    }

    return 1;
}

int KSlave::Stop()
{
    if( running )
    {
	kill( SubProcess, SIGTERM );
	Close();
	close( out );
	close( err );
	running = 0;
	return 1;
    }
    return 0;
}

long KSlave::Write(void *buffer, long len)
{
#ifdef SPECIAL_PIPE_TRICK
	fd_set writefds;
	timeval tv;
	tv.tv_sec = tv.tv_usec = 0;
	FD_ZERO(&writefds);
	FD_SET(in,&writefds);
	if(select(in+1,NULL,&writefds,NULL,&tv) == -1) sleep(1);
#endif
	long rc = write(in,buffer,len);
#ifdef SPECIAL_PIPE_TRICK
	if(rc == -1) printf("SPECIAL_PIPE_TRICK: AMOK write!\n");
	while(rc == -1 && errno == EAGAIN) rc = write(in,buffer,len);
#endif
	return(rc);
}
