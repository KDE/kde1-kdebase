#include <unistd.h>
#include <stdio.h>

/*#define SPECIAL_PIPE_TRICK*/

class KSlave
{
private:
	int BuildPipe(int *from, int *to);

	int running;
	int s_in, s_out, s_err;		// FDs for the slave connected to the FDs below
	int SubProcess;

public:
	const int IN=1, OUT=2, ERR=4;

	int in, out, err;			// FDs to read/write to the slave

	KSlave();
	~KSlave();

	void SetNDelay(int value);
	int Start(char *command);
	long Write(void *buffer, long len);
	int WaitIO(long sec, long usec);
	int Close();
	int Stop();
};
