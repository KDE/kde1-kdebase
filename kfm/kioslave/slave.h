#ifndef _kslave_h
#define _kslave_h

#include <unistd.h>
#include <stdio.h>

/*#define SPECIAL_PIPE_TRICK*/

class KSlave
{
public:
    KSlave();
    ~KSlave();
    
    bool isRunning() { return ( running == 0 ); }
    bool isClosed() { return closed; }
    
    void SetNDelay(int value);
    int Start( const char *command);
    long Write(void *buffer, long len);
    int WaitIO(long sec, long usec);
    int Close();
    int Stop();

    const int IN=1, OUT=2, ERR=4;    
    int in, out, err;			// FDs to read/write to the slave

private:
    int BuildPipe(int *from, int *to);
    
    int running;
    bool closed;
    int s_in, s_out, s_err;		// FDs for the slave connected to the FDs below
    int SubProcess;
};

#endif
