//-----------------------------------------------------------------------------
//
// KDE Help Window Controller (eventually)
//
// (c) Martin R. Jones 1996
//

#ifndef __KHELP_H__
#define __KHELP_H__

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#ifdef __FreeBSD__ || __NetBSD__
#define msgbuf mymsg
#endif

class KHelpMsg : protected msgbuf
{
public:
	int send( int msgqid, int flags = 0 )
		{	return msgsnd( msgqid, this, size(), flags ); }
	int recv( int msgqid, long type, int flags = IPC_NOWAIT )
		{	return msgrcv( msgqid, this, size(), type, flags ); }

	void setCommand( int cmd )
		{	mcommand = cmd; }
	void setType( int type )
		{	mtype = type; }
	void setMsg( const char *_msg )
		{	strcpy( msg, _msg ); }

	int  getType() const
		{	return mtype; }
	const char *getMsg() const
		{	return msg; }

	static int size()
		{	return sizeof(KHelpMsg) - sizeof(long); }
	
private:
	int  mcommand;
	char msg[256];
};


#endif

