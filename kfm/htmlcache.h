#ifndef HTMLCACHE_H
#define HTMLCACHE_H

#include <qstrlist.h>
#include <qdict.h>
#include <qlist.h>
#include <qobject.h>
#include <qstring.h>
#include <qdatetm.h>
#include <qtimer.h>

#include "kiojob.h"

/// This job downloads pages/images from the Web
/**
  This class provides a nice interface for KIOJob.
  So the class is for convenience only.
  */
class HTMLCacheJob : public KIOJob
{
    Q_OBJECT
public:
    HTMLCacheJob( const char *_src_url, const char *_dest_url );
    ~HTMLCacheJob();
    
    const char *getSrcURL() { return url.data(); }
    const char *getDestURL() { return dest.data(); }

    /// Does the real job
    void copy();
    
public slots:
    /// Is called if the job has finished its work
    void slotJobFinished( int _id );

    /**
     * Called if the job makes any progress.
     */
    void slotProgress( int _percent, int _bytesTransered );
    /**
     * Called every 5 secs to check wether the transfer is "stalled".
     */
    void slotProgressTimeout();

signals:
    /// Signals the client that this job is finished
    void finished( HTMLCacheJob * );

    void progress( HTMLCacheJob *, int _percent, int _bytesTrasfered, float _rate, bool _stalled );    

protected:
    QString url;
    QString dest;

    /**
     * Time elapsed since job started.
     */
    QTime timer1;
    /**
     * Time elapsed since last bytes arrived.
     */
    QTime timer2;
    /**
     * Calls us every 5 seconds to check wether the transfer is stalled.
     */
    QTimer *timer;

    /**
     * The amount of bytes transfered right now.
     */
    int bytesTransfered;
    /**
     * The percent of the file that is already downloaded.
     */
    int percent;
};

/// Caches HTML pahes/images
/**
  Every KFileView instance for example has an instance of this class. You can
  tell this class to download pages/images from the web and cache them on the
  local disk. Its signal/slot interface fits the one KHTMLWidget uses.
  The cached files are stored in 'cachePath' which is usually "$HOME/.kfm/cache".
  If two instances request the same page it is only loaded once. So a job running
  in one instance of HTMLCache may server several KHTMLWidgets. This means that the
  design of this class is somewhat wired.
  */
class HTMLCache : public QObject
{
    Q_OBJECT
public:
    /// Creates an interface for the cache.
    /**
      The real cache lives in the static variables and memeber functions. But every
      KHTMLWidget must create an instance of this class to use the signal/slot mechanism.
      */
    HTMLCache();
    /// Delete the cache
    /**
      This will delete all jobs only needed by this instance. But it wont clear the
      cache.
      */
    ~HTMLCache();
    
    /// Deletes all running jobs.
    static void quit();
    
    static QString& getCachePath() { return cachePath; }

    /// This function is called whenever a job has finished its work.
    void urlLoaded( HTMLCacheJob *_job );
    
    /// Returns TRUE if this instance is waiting for '_url'.
    bool waitsForURL( const char *_url );
    
public slots:
    /// The KHTMLWidget may request an URL using this function.
    void slotURLRequest( const char * _url );
    /// This function cancels a request.
    /**
      If two instances of HTMLCache request the same URL and ine instance
      cancels the request, then the job wont be canceled.
      */
    void slotCancelURLRequest( const char * _url );

    /// Called if the job has been finished
    /**
      This slot is bound to HTMLCacheJob. It calls the 'urlLoaded' function of
      all instances of this class. This function exists, because no static slots
      are possible.
      */
    void slotJobFinished( HTMLCacheJob * _job );

    /**
     * This slot is connected to @ref HTMLCacheJob::progress.
     * It emits the signal @ref #progress.
     */
    void slotProgress( HTMLCacheJob *_job, int _percent, int _bytesTransfered,
		       float _rate, bool _stalled );
signals:
    void urlLoaded( const char * _url, const char * _filename );
    
    void progress( const char *_url, const char *_filename,
		   int _percent, int _bytesTrasfered, float _rate, bool _stalled );
    
protected:

    /// The directory in which to store cached data.
    static QString cachePath;

    /// List of all running jobs
    static QList<HTMLCacheJob> htmlJobList;
    
    /// Dict. of all cached URLs
    /**
      urlDict[ "http://www.kde.org/index.html" ] returns a string containing
      the file that holds the cached data.
      */
    static QDict<QString> urlDict;

    /// List of all instances
    static QList<HTMLCache> instanceList;

    /// List of all URLs this instance is waiting fot
    QStrList waitingURLList;
    
    /// This variable holds a new valid and for this process unique fileId.
    static int fileId;
};

#endif
