//----------------------------------------------------------------------------
// Very basic CGI server for kdehelp
//
// This module implements only a fraction of the functionality required
// by a CGI server.  It is really only meant for help searches currently.
//
// Copyright (c) Martin R. Jones 1997

#ifndef __CGI_H__
#define __CGI_H__

#include <qobject.h>
#include <qtimer.h>

class KCGI : public QObject
{
	Q_OBJECT
public:
	KCGI();
	~KCGI();

	bool get( const char *_src, const char *dest, const char *_method );

protected:
	bool runScript();

protected slots:
	void checkScript();

signals:
	void finished();

protected:
	QString query;
	QString script;
	QString pathInfo;
	QString destFile;

	QString method;

	int scriptPID;

	QTimer timer;
};

#endif

