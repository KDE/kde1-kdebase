/**
 * This file contains varoius C functions for
 * almost every purpose.
 */

#ifndef _utils_h
#define _utils_h

#include <qstring.h>
#include <qstrlist.h>

#include <kurl.h>

#include "kbind.h"

/**M
 * Opens the documents in '_urlList' using '_cmd' as command.
 * The files in '_urlList' are URLs or filenames. The filenames
 * are NOT shell quoted, but '_cmd' has to be shell quoted.
 *
 * This command is only useful for old applications which dont
 * understand URLs themselves. If there are URLs, then kfmexec
 * is used to handle the network stuff for this old application.
 * If all files are local, then everything is as usual.
 *
 * The filenames are just appended to '_cmd' if '_cmd' does not
 * conatin "%f" somewhere.
 */
void openWithOldApplication( const char *_cmd, QStrList& _urlList );

QString displayName();

#endif
