#ifndef HTMLCACHE_H
#define HTMLCACHE_H

class HTMLCacheJob;
class HTMLCache;

#include <qstrlist.h>
#include <qdict.h>
#include <qlist.h>
#include <qobject.h>
#include <qstring.h>
#include <qdatetm.h>
#include <qtimer.h>

#include "kiojob.h"

#define MAX_JOBS 4

/**
 * This job downloads pages/images from the Web.
 * This class provides a nice interface for KIOJob.
 * So the class is for convenience only.
 */
class HTMLCacheJob : public KIOJob
{
    Q_OBJECT
public:
    HTMLCacheJob( const char *_src_url, const char *_dest_url );
    ~HTMLCacheJob();
    
    const char *getSrcURL() { return url.data(); }
    const char *getDestURL() { return dest.data(); }

    /**
     * Does the real job.
     */
    void copy();
public slots:
    /**
     * Is called if the job has finished its work.
     */
    void slotJobFinished( int _id );
    void slotError( int _kioerror, const char* _text );
    
signals:
    /**
     * Signals the client that this job is finished.
     */
    void finished( HTMLCacheJob * );
    void error( HTMLCacheJob* _job, int _kioerror, const char *_text );

protected:
    QString url;
    QString dest;
};

/**
 * @short Caches HTML pahes/images
 *
 * Every KFileView instance for example has an instance of this class. You can
 * tell this class to download pages/images from the web and cache them on the
 * local disk. Its signal/slot interface fits the one KHTMLWidget uses.
 * The cached files are stored in 'cachePath' which is usually "$HOME/.kfm/cache".
 * If two instances request the same page it is only loaded once. So a job running
 * in one instance of HTMLCache may server several KHTMLWidgets. This means that the
 * design of this class is somewhat wired.
 */
class HTMLCache : public QObject
{
    Q_OBJECT
public:
    /**
     * Creates an interface for the cache.
     * The real cache lives in the static variables and memeber functions. But every
     * KHTMLWidget must create an instance of this class to use the signal/slot mechanism.
     */
    HTMLCache();
    /**
     * Delete the cache.
     * This will delete all jobs only needed by this instance. But it wont clear the
     * cache.
     */
    ~HTMLCache();
    
    /**
     * Deletes all running jobs.
     */
    static void quit();
    
    static QString& getCachePath() { return (*cachePath); }

    /**
     * This function is called whenever a job has finished its work.
     */
    void urlLoaded( HTMLCacheJob *_job );
    
    /**
     * @return TRUE if this instance is waiting for '_url'.
     */
    bool waitsForURL( const char *_url );

    /**
     * @return 0L if the URL is not cached, or the filename ( starting with '/' )
     *         of the file, which holds the cached data.
     */
    const char* HTMLCache::isCached( const char *_url );
    
    /**
     * Cancels all jobs started by this instance.
     */
    void stop();

    /**
     * Writes the contents of @ref urlDict to disk
     */
    static void save();
    /**
     * Tries to load the file written by @ref #save
     */
    static void load();
    /**
     * Tries to clear the complete cache.
     */
    static void clear();
    /**
     * Enables or disbales the cache.
     */
    static void enableCache( bool _enable );
    static void enableSaveCache( bool _enable );
    /**
     * Check enable status
     */
    static bool isEnabled();
    static bool isSaveEnabled();
    
    /*
     * hack to get static classes up and running even with C++-Compilers/
     * Systems where a constructor of a class element declared static
     * would never get called (Aix, Alpha,...). In the next versions these
     * elements should disappear.
     */
    static void InitStatic() {
    	cachePath = new QString;
    	staticJobList = new QList<HTMLCacheJob>;
    	urlDict = new QDict<QString>;
    	instanceList = new QList<HTMLCache>;
    }

public slots:
    /**
     * Checks in an already loaded URL.
     */
    void slotCheckinURL( const char *_url, const char *_data );
    /**
     * The KHTMLWidget may request an URL using this function.
     */
    void slotURLRequest( const char * _url );
    /**
     * This function cancels a request.
     * If two instances of HTMLCache request the same URL and ine instance
     * cancels the request, then the job wont be canceled.
     */
    void slotCancelURLRequest( const char * _url );

    /** Called if the job has been finished
     * This slot is bound to HTMLCacheJob. It calls the 'urlLoaded' function of
     * all instances of this class. This function exists, because no static slots
     * are possible.
     */
    void slotJobFinished( HTMLCacheJob * _job );

    void slotError( HTMLCacheJob *_job, int _kioerror, const char *_text );
    
signals:
    void urlLoaded( const char * _url, const char * _filename );
    
    void progress( const char *_url, const char *_filename,
		   int _percent, int _bytesTrasfered, float _rate, bool _stalled );
    
protected:

    /**
     * The directory in which to store cached data.
     */
    static QString *cachePath;

    /**
     * List of all running jobs
     */
    static QList<HTMLCacheJob> *staticJobList;
    
    /**
     * Dict. of all cached URLs.
     * urlDict[ "http://www.kde.org/index.html" ] returns a string containing
     * the full path of the file that holds the cached data.
     */
    static QDict<QString> *urlDict;

    /**
     * If the cache is disabled, then the value of this member is set to
     * false and it is not allowed to lookup entries in @ref #urlDict.
     *
     * @see #enableCache
     */
    static bool bCacheEnabled;
    static bool bSaveCacheEnabled;
    
    /**
     * List of all instances
     */
    static QList<HTMLCache> *instanceList;

    /**
     * List of all URLs this instance is waiting for.
     */
    QStrList waitingURLList;
    
    /**
     * This variable holds a new valid and for this process unique fileId.
     */
    static int fileId;

    /**
     * List of all requests which could not be started yet since no
     * job was available.
     */
    QStrList todoURLList;

    /**
     * List of all jobs startd by this instance. This is always a
     * subset of the jobs in @ref #staticJobList.
     */
    QList<HTMLCacheJob> instanceJobList;
};

#endif




