#ifndef KBIND_H
#define KBIND_H

class KMimeType;
class KMimeBind;
class KFMConfig;

#include <qstring.h>
#include <qlist.h>
#include <qstrlist.h>
#include <qfile.h>

#include <kurl.h>
#include <kapp.h>
#include <Kconfig.h>

#include "kioserver.h"

// A Hack, since KConfig has no constructor taking only a filename
class KFMConfig : public KConfig
{
public:
    KFMConfig( QFile *, QTextStream* );
    ~KFMConfig();
    
protected:
    QTextStream *pstream;
    QFile *f;
};


/**
 * @short The application bindings.
 *
 * For every KMimeType you can create multiple bindings. These binding
 * contain the name of the program, the command to start it and
 * the protocols supported.
 */
class KMimeBind
{
public:
    /**
     * Creates a FileBinding with the name as 1st parameter. The 2nd
     * parameter is the command line. You may use '%f' as a wildcard for the path
     * (makes only sense for the protocol 'file') or '%u' for the URL of a file.
     * The last 3 parameters conatain one or more supported protocols. A Web Browser
     * will for example support 'file', 'http', 'ftp' while a graphics program will
     * only support 'file'. While the WWW Browser will use '%u', the graphics program
     * will use '%f'.
     */
    KMimeBind( const char *_name, const char *_cmd, bool _allowdefault, const char *_prot1,
	       const char *_prot2 = 0, const char * _prot3 = 0,
	       const char* _prot4 = 0, const char * _prot5 = 0 );
    ~KMimeBind() {}
    
    /**
     * @return the programs name
     */
    const char* getProgram() { return (const char*)program; }
    /**
     * @eturn the command string.
     */
    const char* getCmd() { return (const char*)cmd; }

    /**
     * @return TRUE if this application is allowed as default app.
     *         The default app is started if the user clicks on a
     *         data file.
     */
    bool isAllowedAsDefault() {	return allowDefault; }
    
    /**
     * Tests wether this file binding supports the specified protocol.
     */
    bool supportsProtocol( const char *_protocol );

    /**
     * Register another application.
     * Every application is registered using this function. This way we have
     * a list of all registered applications.
     */
    static void appendApplication( const char *_appname ) { appList.append( _appname ); }
    
    /**
     * @return the name of the first registered application.
     */
    static const char* getFirstApplication() { return appList.first(); }
    /**
     * Use this function only after a call to @ref #getFirstApplication.
     *
     * @return the name of the next registered application.
     */
    static const char* getNextApplication() { return appList.next(); }
    /**
     * Clear the list of registered applications.
     * This function is used if the user changes some of the application/MimeType
     * entries and they have to be refreshed.
     */
    static void clearApplicationList() { appList.clear(); }
    
protected:
    /**
     * The programs name.
     */
    QString program;
    /**
     * The command string.
     */
    QString cmd;
    /**
     * First supported protocol. May not be null.
     */
    QString protocol1;
    /**
     * Second supported protocol. May be null.
     */
    QString protocol2;
    /**
     * Third supported protocol. May be null.
     */
    QString protocol3;
    /**
     * Fourth supported protocol. May be null.
     */
    QString protocol4;
    /**
     * Fives supported protocol. May be null.
     */
    QString protocol5;

    /**
     * List of all registered applications.
     *
     * @see #appendApplication
     */
    static QStrList appList;

    /**
     * @see #IsAllowedAsDefault
     */
    bool allowDefault;
};

/**
 * For every mime type
 * you can create a KMimeType. This way the program knows which
 * icon to use and which programs can handle the data.
 * Have a look at KMimeBind. Multiple extensions can be organized
 * in KMimeType ( for example *.tgz, *.tar.gz ) since they
 * mean the same document class.
 *
 * @see KMimeBind
 */
class KMimeType
{
public:
    /**
     * Create a mime type. You must later on call @setPixmap and
     * @setPattern and so on to fill the object with data.
     */
    KMimeType() { }
    /**
     * Create a mime type and give it an icon.
     */
    KMimeType( const char *_mime_type, const char *_pixmap );

    /**
     * Add a binding to this type.
     */
    virtual void append( KMimeBind * );

    /**
     * Set the icon.
     */
    virtual void setPixmap( const char * );
    /**
     * Add a pattern which matches this type. Patterns are for example
     * "*.tgz" or "*.jpg".
     */
    virtual void addPattern( const char *_p ) { if ( pattern.find( _p ) == -1 ) pattern.append( _p ); }

    /**
     * Set the default bindings name. If the user just clicks on a document then
     * we try to execute the default binding ( read: application ).
     *
     * @param _b is the name of a *.kdelnk file in the $KDEDIR/apps tree. '_b' does NOT
     *           include the ".kdelnk" suffix.
     */
    virtual void setDefaultBinding( const char *_b ) { defaultBinding = _b; defaultBinding.detach(); }

    /**
     * @return the extensions assoziated with this MimeType, for
     *         example '.html' or '.cpp'.
     */
    virtual QStrList& getPattern() { return pattern; }

    /**
     * The mime type in the file '$KDEDIR/MimeTypes/text+html.kdelnk' for
     * example is called 'text/html'. The '+' in the filename is becomes
     * a '/' and the suffix ".kdelnk" is deleted. That is what this function
     * returns
     *
     * @return the name of this mime type. This name may be used by an application
     *         to define the kind of data it works on.
     */
    const char* getMimeType() { return mimeType.data(); }
    
    /**
     * The return vakue may be 0L if there is no comment at all.
     * This method does not use _url but some overloading methods do so.
     *
     * @return this file types comment.
     */
    virtual QString getComment( const char * ) { return QString( comment.data() ); }

    /**
     * Sets the comment.
     */
    void setComment( const char *_c) { comment = _c; }

    /**
     * Set the mimetype of all files matching this MimeTypes pattern.
     */
    void setMimeType( const char *_m ) { mimeType= _m; }

    /**
     * Marks this file type to be only an application pattern.
     * Since it is possible to give binaries an icon, we must create a mime type
     * for those binaries. This functions sets a flag that indicates that this is not
     * a real mime type.
     */
    void setApplicationPattern() { bApplicationPattern = TRUE; }
    /**
     * Tells wether this file type is only an application pattern.
     *
     * @see #setApplicationPattern for details.
     */
    bool isApplicationPattern() { return bApplicationPattern; }
    
    /**
     * @return the full qualified filename ( not an URL ) for the icons
     *         pixmap.
     */
    virtual const char* getPixmapFile( const char *_url );
    /**
     * @returns the pixmap that is associated with this kind of
     *          mime type.
     */
    virtual QPixmap& getPixmap( const char *_url );
    /**
     * @return a pointer to the default binding.
     *         The return may be 0. In this case the user did not specify
     *         a special default binding. That does not mean that there is no
     *         binding at all.
     */
    virtual const char* getDefaultBinding() { return defaultBinding; }
    
    /**
     * Tells wether bindings are available.
     */
    virtual bool hasBindings() { return !bindings.isEmpty(); }
    /**
     * Gets the first binding if available. Dont use 'firstBinding' and
     * @ref #nextBinding in two cascading loops at once.
     */
    virtual KMimeBind* firstBinding() { return bindings.first(); }
    /**
     * Gets the next binding if available. Dont use @ref #firstBinding and
     * 'nextBinding' in two cascading loops at once.
     * Call @ref #firstBinding before calling this function.
     */
    virtual KMimeBind* nextBinding() { return bindings.next(); }

    /**
     * Find a binding by its name.
     */
    virtual KMimeBind* findBinding( const char *_filename );
	
    /**
     * Tries to find a MimeType for the file _url. If no special
     * MimeType is found, the default MimeType is returned.
     * This function only looks at the filename not at the content
     * of the file.
     */
    static KMimeType* findType( const char *_url );

    /**
     * Find type by pattern.
     * If for the given pattern no KMimeType has been created, the function
     * will retun 0L;
     */
    static KMimeType *findByPattern( const char *_pattern );
    
    /**
     * Finds a file type by its name
     * The file type in the file '$KDEDIR/MimeTypes/text+html.kdelnk' for
     * example is called 'text/html'.
     */
    static KMimeType *findByName( const char *_name );
    
    /**
     * Fills _list with all bindings for _url. Each binding is 
     * represented by a string. You may use such a string as text
     * of an item in a popup menu.
     * With @ref #findByName you can get the @ref KMimeType object associated
     * with the name.
     *
     */
    static void getBindings( QStrList &_list, const char *_url, bool _isdir );
    /**
     * Tries to execute the binding named _binding for the file _url.
     * The default binding is "Open". For example when the user double clicks
     * on something and you dont know a specific binding, then select "Open".
     * If you want to pass some URLs to an executable ( for examples documents to open )
     * then you can pass a list of all URLs in '_args'. This makes sense only with 
     * executables or *.kdelnk files of the type "Exec".
     * '_url' may be an executable, a directory, or just a document.
     * If it is a directory or a document then the '_args' dont matter. If in the other case
     * '_url' is an executable then '_binding' is not used.
     */
    static void runBinding( const char *_url, const char *_binding, QStrList * _args = 0L );
    /**
     * Runs the default binding.
     * If there is only one binding, this one is executed otherwise the default
     * binding is executed. If there are multiple bindings and none of them is the default
     * one, the first one is executed. Usually used when the user clicks
     * on an URL. Files named *.kdelnk are executed, too.
     * This function only determines the default binding and calls 'runBinding( .., .. )'
     * to do the real job. If no binding can be found, the binding "Open" is given a try.
     * Note that this function does NOT execute bindings which are not allowed as
     * default bindings ( @ref KMimeBind::isAllowedAsDefault ).
     */
    static void runBinding( const char *_url );

    /**
     * Open a KConfig file
     * @return a KConfig if the file starts with "[KDE Desktop Entry]" otherwise 0L.
     *         The group "[KDE Desktop Entry"] will be already selected in the returned
     *         KConfig object.
     */
    static KFMConfig* openKFMConfig( const char *_url );

    /**
     * @eturn the first mime type or 0L if there is no mime type at all.
     */
    static KMimeType *getFirstMimeType();
    /**
     * Use this function only after a call to @ref #getFirst.
     *
     * @return the next mime type or 0L if there are no more file types.
     */
    static KMimeType *getNextMimeType();

    /**
     * This function deletes all file bindings and types.
     * This is done if the user changed some application/MimeType settings.
     */
    static void clearAll();
    
    /**
     * Call this function before you call any other function. This
     * function initializes some global variables.
     */
    static void init();
    
    /**
     * Scan the $KDEDIR/apps directory for application bindings
     */
    static void initApplications( const char *_path );

    /**
     * Scan the $KDEDIR/MimeTypes directory for the mime types
     */
    static void initMimeTypes( const char *_path );
    
    /**
     * @return the path for the icons
     */
    static const char* getIconPath() { return icon_path; }
    static const char* getDefaultPixmap() { return defaultPixmap; }    
    static const char* getExecutablePixmap() { return executablePixmap; }    
    static const char* getBatchPixmap() { return batchPixmap; }    
    static const char* getFolderPixmap() { return folderPixmap; }    
    static const char* getLockedFolderPixmap() { return lockedfolderPixmap; }
    static const char* getPipePixmap() { return PipePixmap; }
    static const char* getSocketPixmap() { return SocketPixmap; }
    static const char* getCDevPixmap() { return CDevPixmap; }
    static const char* getBDevPixmap() { return BDevPixmap; }         

    /**
     * Runs the given command using fork.
     *
     * @param _cmd is a command like "kedit ftp://weis@localhost/index.html" or
     *             something like that.
     *
     * @see #runBinding
     */
    static void runCmd( const char *_cmd );

protected:    
    /**
     * List of all bindings for this type.
     */
    QList<KMimeBind> bindings;

    /**
     * The full qualified filename ( not an URL ) of the icons
     * pixmap.
     */
    QString pixmap_file;
    /**
     * The pixmap used for the icon.
     */
    QPixmap pixmap;

    /**
     * The pattern matching this file. For example: "*.html".
     */
    QStrList pattern;

    /**
     * Holds the default binding.
     * This string may be 0. In this case the user did not specify
     * a special default binding. That does not mean that there is no
     * binding at all. Attention: Perhaps a binding with that name does
     * not exist for a strange reason.
     */
    QString defaultBinding;
    
    /**
     * Holds a comment to this file type.
     */
    QString comment;

    /**
     * Holds the mime type. Something like "text/html".
     */
    QString mimeType;
    
    /**
     * The path to the icons.
     */
    static char icon_path[ 1024 ];
    /**
     * Default pixmap for executables.
     */
    static char executablePixmap[ 1024 ];
    /**
     * Default pixmap for batch files.
     */
    static char batchPixmap[ 1024 ];
    /**
     * General default pixmap.
     */
    static char defaultPixmap[ 1024 ];
    /**
     * Default pixmap for folders.
     */
    static char folderPixmap[ 1024 ];    
    /**
     * Default pixmap for locked folders
     */
    static char lockedfolderPixmap[ 1024 ];
    /**
     * Default pixmap for Pipes
     */
    static char PipePixmap[ 1024 ];
    /**
     * Default pixmap for Sockets
     */
    static char SocketPixmap[ 1024 ];
    /**
     * Default pixmap for a character device
     */
    static char CDevPixmap[ 1024 ];
    /**
     * Default pixmap for a block device
     */
    static char BDevPixmap[ 1024 ];        

    /**
     * This flag is set if this file type is only an application pattern.
     *
     * @see #setApplicationPattern for details.
     */
    bool bApplicationPattern;
};

/**
 * @sort A special KMimeType for folders.
 *
 * Looks in the folder for a file called '.desktop'. This file may include
 * an icon entry. Otherwise the default icon is used.
 */
class KFolderType : public KMimeType
{
public:
    KFolderType() : KMimeType() { }

    /**
     * Get the pixmap files full name.
     * WARNING: This function is NOT reentrant. Copy the returned
     * string before calling the function again.
     */
    virtual const char* getPixmapFile( const char *_url );

    /**
     * Get the pixmap for the given URL.
     * Returns the pixmap that is associated with this kind of
     * MimeType. The information about this pixmap is taken from
     * the K Comments in the *.kdelnk file.
     * WARNING: This function is NOT reentrant. Copy the returned
     * pixmap before calling the function again.
     */
    virtual QPixmap& getPixmap( const char *_url );

    /**
     * This function reads the comment in the ".directory" file
     * if available.
     */
    virtual QString getComment( const char *_url );
    
protected:

    /**
     * The pixmaps full filename.
     * When a directory contains a '.directory' file, then the corresponding
     * icon filename is stored here. Since another call to @ref #getPixmap may overwrite
     * this variable, this causes the whole stuff to be NOT reentrant.
     */
    QString pixmapFile2;
};


/**
 * @short This class is used for all files with the ending *.kdelnk
 *
 * The *.kdelnk files may include an icon entry.
 */
class KDELnkMimeType : public KMimeType
{
public:
    /**
     * Create a KDELnkMimeType. You need not specify pixmaps or
     * patterns. All information is in the *.kdelnk files.
     */
    KDELnkMimeType() : KMimeType() { }

    /**
     * WARNING: This function is NOT reentrant. Copy the returned
     * string before calling the function again.
     */
    virtual const char* getPixmapFile( const char *_url );

    /**
     * Returns the pixmap that is associated with this kind of
     * MimeType. The information about this pixmap is taken from
     * the K Comments in the *.kdelnk file.
     * WARNING: This function is NOT reentrant. Copy the returned
     * pixmap before calling the function again.
     */
    virtual QPixmap& getPixmap( const char *_url );

    /**
     * The return value may be 0L if there is no comment at all.
     * The comment is taken from the *.kdelnk file '_url.'
     *
     * @returns this file types comment.
     */
    virtual QString getComment( const char *_url );
};

#endif





