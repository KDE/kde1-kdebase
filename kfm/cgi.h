#ifndef __CGI_H__
#define __CGI_H__

#include <qobject.h>
#include <qtimer.h>

/**
 * @short Very basic CGI server
 *
 * This module implements only a fraction of the functionality required
 * by a CGI server. Copyright (c) Martin R. Jones 1997
 */
class KCGI : public QObject
{
    Q_OBJECT
public:
    KCGI();
    ~KCGI();

    /**
     * Runs a local script and returns before! the script finished.
     * Connect to @ref #finished to get informed abou the result.
     *
     * @param _script is the script and its parameters. For example
     *                <tt>serach.cgi?argument=FindMe</tt>
     *
     * @return the filename of the output. If an error occures
     *         that can not be handled, an empty String is returned.
     */
    QString get( const char *_script, const char *_method );

    /**
     * Stops a running script. Not implemented yet.
     */
    void stop();
    
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

