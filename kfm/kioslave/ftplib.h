// Taken from the original libftp on sunsite Linux/libs

#ifndef ftplib_h
#define ftplib_h

#include <stdio.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "kioslave_ipc.h"

int ftpOpen( const char *host);
int ftpLogin( const char *user, const char *pass);
int ftpSite( const char *cmd);
int ftpMkdir( const char *path);
int ftpChdir( const char *path);
int ftpRmdir( const char *path);
int ftpNlst( const char *output, const char *path);
int ftpDir( FILE *output, const char *path);
int ftpGet( FILE *output, const char *path, char mode, KIOSlaveIPC * ipc );
int ftpPut( FILE *input, const char *path, char mode, KIOSlaveIPC * ipc, int size );
int ftpRename( const char *src, const char *dst);
int ftpDelete( const char *fnm);
void ftpQuit(void);

#endif
