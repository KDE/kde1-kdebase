#ifndef KBIND_H
#define KBIND_H

class KMimeType;
class KMimeBind;

#include <qstring.h>
#include <qlist.h>
#include <qstrlist.h>
#include <qfile.h>
#include <qpmcache.h>

#include <kurl.h>
#include <kapp.h>
#include <kconfig.h>
#include <ksimpleconfig.h>

#include "kioserver.h"
#include "kmimemagic.h"

extern KMimeType *defaultType;
extern KMimeType *kdelnkType;
extern KMimeType *folderType;
extern KMimeType *execType;
extern KMimeType *batchType;
extern KMimeType *lockedfolderType;
extern KMimeType *PipeType;
extern KMimeType *SocketType;
extern KMimeType *CDevType;
extern KMimeType *BDevType;   

/**
 * @short The application bindings.
 *
 * For every @ref KMimeType you can create multiple bindings. These binding
 * contain the name of the program, the command to start it and
 * the protocols supported.
 */
class KMimeBind
{
public:
    /**
     * Creates a FileBinding
     * @param _kdelnkName the name of the application kdelnk
     * @param _name the name (contents of the Name= field, possibly translated).
     * @param _cmd the command line. You may use '%f' as a wildcard for the path
     *   (makes only sense for the protocol 'file') or '%u' for the URL of a file.
     *   The last 3 parameters contain one or more supported protocols. A Web Browser
     *   will for example support 'file', 'http', 'ftp' while a graphics program will
     *   only support 'file'. While the WWW Browser will use '%u', the graphics program
     *   will use '%f'.
     * @param _pixmap the pixmap defined by Icon= in the app. kdelnk
     * @param _allowdefault whether this can be a default binding (see AllowDefault= field)
     * @param _termOptions 0L if doesn't run in a terminal, otherwise the terminal options
     */
    KMimeBind( const char *_kdelnkName, const char *_name, const char *_cmd, const char *_pixmap, bool _allowdefault, const char *_termOptions );
    virtual ~KMimeBind() {}
    
    /**
     * @return the program's name (as defined in the app. kdelnk)
     */
    const char* getName() { return (const char*)name; }
    /**
     * @eturn the command string.
     */
    const char* getCmd() { return (const char*)cmd; }
    /**
     * @return the app. kdelnk's name
     */
    const char* getKdelnkName() { return (const char*)kdelnkName; }

    /**
     * @return a pointer to the pixmap for this application.
     *         The pointer is at no time 0L, but the pixmap may be empty.
     */
    QPixmap* getPixmap( bool _mini );
    
    /**
     * @return TRUE if this application is allowed as default app.
     *         The default app is started if the user clicks on a
     *         data file. For example you may have a binding to compress large
     *         image file, but you dont want this binding to be executed if
     *         you click on the corresponding icon. By clearing the allowDefault
     *         flag you can prevent this.
     */
    bool isAllowedAsDefault() {	return allowDefault; }
    
    /**
     * Uses this binding ( read: application ) to open the document '_url'.
     */
    virtual bool runBinding( const char *_url );

    /**
     * Register another binding.
     * Every application is registered using this function. This way we have
     * a list of all registered applications. This list is needed in the properties
     * for example.
     *
     * @see #appList
     */
    static void appendBinding( KMimeBind *_bind ) { s_lstBindings->append( _bind ); }
    
    /**
     * @return the name of the first registered application.
     *
     * @see #appendBinding
     */
    static QListIterator<KMimeBind> bindingIterator();
    /**
     * Clear the list of registered applications.
     * This function is used if the user changes some of the application/MimeType
     * entries and they have to be refreshed.
     *
     * @see #appendBinding
     */
    static void clearBindingList() { s_lstBindings->clear(); }

    static KMimeBind* findByName( const char *_name );
  
    /**
     * This function tries to find out about the mime type of the URL.
     * It then searches the matching binding and tells the binding
     * to open the document. This is used for example if you know the
     * binding is "KView" and the URL is "file:/tmp/image.gif". This
     * happens for example if you deal with context sensitive popup menus
     * like @ref KFMManager does. This is just a very simple
     * convenience function.
     *
     * @return TRUE if the function knew what to do with the URL
     */
    static bool runBinding( const char *_url, const char *_binding );

    /**
     * Runs the given command using fork.
     *
     * @param _cmd is a command like "kedit ftp://weis@localhost/index.html" or
     *             something like that.
     * @param _workdir is the optionnal working directory
     *
     * @see #runBinding
     */
    static void runCmd( const char *_cmd, const char *_workdir = 0L );

    /**
     * Another interface to the 'exec' clib functions.
     */
    static void runCmd( const char *_exec, QStrList &_args, const char *_workdir = 0L );

    /**
     * Open a config file
     * @return a @ref KSimpleConfig if the file starts with "[KDE Desktop Entry]" otherwise 0L.
     *         The group "[KDE Desktop Entry"] will be already selected in the returned config object.
     */
    static KSimpleConfig* openKConfig( const char *_url );

    /**
     * Scan the $KDEDIR/apps directory for application bindings
     */
    static void initApplications( const char *_path );

    /*
     * hack to get static classes up and running even with C++-Compilers/
     * Systems where a constructor of a class element declared static
     * would never get called (Aix, Alpha,...). In the next versions these
     * elements should disappear.
     */
    static void InitStatic();

protected:
    /**
     * The (possibly translated) name, as set by the 'Name=' field.
     */
    QString name;
    /**
     * The name of the application kdelnk
     */
    QString kdelnkName;
    /**
     * The command string.
     */
    QString cmd;
    QString pixmapFile;
    QString miniPixmapFile;
    QPixmap *pixmap;
    
    /**
     * List of all bindings.
     *
     * @see #appendBinding
     */
    static QList<KMimeBind>* s_lstBindings;
    
    /**
     * @see #IsAllowedAsDefault
     */
    bool allowDefault;

    /*
     * Terminal options (0L if not terminal used)
     */
    QString termOptions;
};

/**
 * For every mime type
 * you can create a KMimeType. This way the program knows which
 * icon to use and which programs can handle the data.
 * Have a look at @ref KMimeBind. Multiple extensions can be organized
 * in KMimeType ( for example *.tgz, *.tar.gz ) since they
 * mean the same document class.
 *
 * @see KMimeBind
 */
class KMimeType
{
public:
    /**
     * Create a mime type and give it an icon.
     */
    KMimeType( const char *_mime_type, const char *_pixmap );

  virtual ~KMimeType() {};

    /**
     * Add a binding to this type.
     */
    virtual void append( KMimeBind * );

    /**
     * Remove a binding from this type.
     */
    virtual void remove( KMimeBind * b) { bindings.removeRef(b); }

    /**
     * Add a pattern which matches this type. Patterns are for example
     * "*.tgz" or "*.jpg".
     */
    virtual void addPattern( const char *_p ) { if ( pattern.find( _p ) == -1 ) pattern.append( _p ); }

    /**
     * Set the default bindings name. If the user just clicks on a document then
     * we try to execute the default binding ( read: application ).
     *
     * @param _b is the name of a *.kdelnk file in the $KDEDIR/applnk tree. '_b' does NOT
     *           include the ".kdelnk" suffix.
     */
    virtual void setDefaultBinding( const char *_b ) { defaultBinding = _b; defaultBinding.detach(); }

    /**
     * @return the extensions assoziated with this MimeType, for
     *         example '.html' or '.cpp'.
     */
    virtual QStrList& getPattern() { return pattern; }

    /**
     * The mime type in the file '$KDEDIR/mimelnk/text/html.kdelnk' for
     * example is called 'text/html'. The suffix ".kdelnk" is deleted.
     * That is what this function returns
     *
     * @return the name of this mime type. This name may be used by an application
     *         to define the kind of data it works on. An example is "text/html".
     */
    const char* getMimeType() { return mimeType.data(); }
    /**
     * Set the mimetype of all files matching this MimeTypes pattern.
     *
     * @see #getMimeType
     */
    void setMimeType( const char *_m ) { mimeType= _m; }
    
    /**
     * The return value may be empty if there is no comment at all.
     * This method does not use the parameter, but some overloading methods do so.
     *
     * @return this file types comment.
     */
    virtual QString getComment( const char * ) { return QString( comment.data() ); }
    /**
     * Sets the comment.
     *
     * @see #getComment
     */
    void setComment( const char *_c) { comment = _c; }

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
     *         pixmap. For example *.kdelnk files icon depends on
     *         the content of the *.kdelnk file => You must tell
     *         about the URL if you want this feature.
     */
    virtual const char* getPixmapFile( const char *_url, bool _mini = FALSE )
        {   return getPixmapFile( _mini ); }
    /**
     * This function returns the icon associated with this mime type. It returns
     * a full qualified filename.
     */
    const char* getPixmapFile( bool _mini = FALSE );
    /**
     * @returns the pixmap that is associated with this kind of
     *          mime type.
     */
    virtual QPixmap* getPixmap( const char *_url, bool _mini = FALSE );
    /**
     * @return a pointer to the default binding.
     *         The return may be 0. In this case the user did not specify
     *         a special default binding. That does not mean that there is no
     *         binding at all.
     *
     * @see #defaultBinding
     */
    virtual const char* getDefaultBinding() { return defaultBinding; }

    /**
     * Call this function to execute the best fitting binding for '_url'. Since this is
     * no static function, you already made a guess about the mime type of the URL.
     * We look out for the default binding, otherwise we try to execute the first
     * binding that is allowed to be a default. If none of the above succeeds
     * we return FALSE, otherwise TRUE.
     * Use @ref #findByName to get the correct @ref KMimeType instance if you already
     * know the mime type, for example "text/html". Then you may call @ref #run to
     * execute the URL.
     *
     * @see #runBinding
     * @see #defaultBinding
     * @see #bindings
     */
    virtual bool run( const char *_url );
    
    /**
     * Compareable to @ref #run, but we know the name of the binding.
     */
    virtual bool runBinding( const char *_url, const char *_binding );
    
    /**
     * This function is overloaded by @ref ExecutableMimeType and
     * @ref KDELnkMimeType. For usual documents is does not make sense
     * since they are no applications :-)
     */
    virtual bool runAsApplication( const char *_url, QStrList *_arguments );
    
    /**
     * Tells wether bindings are available.
     *
     * @see #bindings
     */
    virtual bool hasBindings() { return !bindings.isEmpty(); }
    /**
     * Gets the first binding if available. Dont use 'firstBinding' and
     * @ref #nextBinding in two cascading loops at once.
     *
     * @see #bindings
     */
    virtual KMimeBind* firstBinding() { return bindings.first(); }
    /**
     * Gets the next binding if available. Dont use @ref #firstBinding and
     * 'nextBinding' in two cascading loops at once.
     * Call @ref #firstBinding before calling this function.
     *
     * @see #bindings
     */
    virtual KMimeBind* nextBinding() { return bindings.next(); }

    /**
     * Find a binding by its kdelnk name.
     */
    virtual KMimeBind* findBinding( const char *_kdelnkName );

    /**
     * @return TRUE if we dont know anything about the file and have
     *         to assume the default mime type.
     */
    virtual bool isDefault();

    /**
     * Find type by pattern.
     * If for the given pattern no KMimeType has been created, the function
     * will retun 0L;
     */
    static KMimeType *findByPattern( const char *_pattern );
    
    /**
     * Finds a mime type by its name
     * The file type in the file '$KDEDIR/mimelnk/text/html.kdelnk' for
     * example is called 'text/html'.
     */
    static KMimeType *findByName( const char *_name );

    /**
     * Used to determine the pixmap for a certain file.
     * Uses @ref KMimeMagic for this.
     *
     * @see #getMagicMimeType
     */
    static const char* getPixmapFileStatic( const char *_filename, bool _mini = FALSE );

    /**
     * Used to determine the mime type of a given file on the local
     * hard disk.
     */
    static KMimeType* getMagicMimeType( const char *_filename );

    /**
     * Fills _list with all bindings for _url. Each binding is 
     * represented by a string. You may use such a string as text
     * of an item in a popup menu.
     * With @ref #findByName you can get the @ref KMimeType object associated
     * with the name.
     *
     */
    static void getBindings( QStrList &_list, QList<QPixmap> &_pixlist, const char *_url, int _isdir );
    /**
     * This function runs the given binding with the URL.
     */
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
     * Scan the $KDEDIR/mimelnk directory for the mime types
     */
    static void initMimeTypes( const char *_path );

    /**
     * Prints out an error and exits if the mime type '_type'
     * is missing but required.
     *
     * @param _ptr is filled with some default mime type to prevent
     *             KFM from segfaulting.
     */
    static void errorMissingMimeType( const char *_type, KMimeType **_ptr );

    /**
     * @return the path for the specific icon.
     */
    static QString getIconPath( const char *_icon, bool _mini = false );

    static const char* getDefaultPixmap() { return "unknown.xpm"; }

    static QPixmapCache* pixmapCache;
   
    /**
     * Initializes the mime type detection module.
     */
    static void initKMimeMagic();
    /**
     * @return a pointer to the mime type detection module.
     */
    static KMimeMagic* getKMimeMagic() { return magic; }
    /**
     * Query the mime type detection module.
     *
     * @param _filename must be a valid file/directory on the
     *                  local hard disk. It may NOT contain a
     *                  protocol, since it is NOT an URL.
     */
    static KMimeMagicResult* findFileType( const char *_filename ) 
    {
	return magic->findFileType( _filename );
    }
    /**
     * Tries to detect the mime type by looking at some sample string
     * of given length.
     */
    static KMimeMagicResult* findBufferType( const char *_sample, int _len ) 
    {
	return magic->findBufferType( _sample, _len );
    }

    /*
     * hack to get static classes up and running even with C++-Compilers/
     * Systems where a constructor of a class element declared static
     * would never get called (Aix, Alpha,...). In the next versions these
     * elements should disappear.
     */
    static void InitStatic();

protected:    
    /**
     * Tries to find a MimeType for the file _url. If no special
     * MimeType is found, the default MimeType is returned.
     * This function only looks at the filename not at the content
     * of the file.
     * I made it private since getMagicMimeType should always
     * be called from the other classes (David).
     */
    static KMimeType* findType( const char *_url );

    /**
     * List of all bindings for this type.
     */
    QList<KMimeBind> bindings;

    /**
     * The full qualified filename ( not an URL ) of the icons
     * pixmap.
     */
    QString pixmapFile;
    QString miniPixmapFile;

    /**
     * The name of the icon, without path.
     */
    QString pixmapName;
    
    /**
     * The pixmap used for the icon.
     */
    QPixmap *pixmap;

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
     * This flag is set if this file type is only an application pattern.
     *
     * @see #setApplicationPattern for details.
     */
    bool bApplicationPattern;

    static KMimeMagic *magic;
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
    KFolderType( const char *_mime_type, const char *_pixmap ) :
	KMimeType( _mime_type, _pixmap ) { }

  virtual ~KFolderType() {};

    /**
     * Get the pixmap files full name.
     * WARNING: This function is NOT reentrant. Copy the returned
     * string before calling the function again.
     */
    virtual const char* getPixmapFile( const char *_url, bool _mini = FALSE );

    /**
     * Get the pixmap for the given URL.
     * Returns the pixmap that is associated with this kind of
     * MimeType. The information about this pixmap is taken from
     * the K Comments in the *.kdelnk file.
     * WARNING: This function is NOT reentrant. Copy the returned
     * pixmap before calling the function again.
     */
    virtual QPixmap* getPixmap( const char *_url, bool _mini = FALSE );

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
     * Create a KDELnkMimeType.
     */
    KDELnkMimeType( const char *_mime_type, const char *_pixmap ) :
	KMimeType( _mime_type, _pixmap ) { }

  virtual ~KDELnkMimeType() {};

    virtual bool run( const char *_url );
    
    /**
     * Compareable to @ref #run, but we know the name of the binding.
     */
    virtual bool runBinding( const char *_url, const char *_binding );

    virtual bool runAsApplication( const char *_url, QStrList *_arguments );

    /**
     * WARNING: This function is NOT reentrant. Copy the returned
     * string before calling the function again.
     */
    virtual const char* getPixmapFile( const char *_url, bool _mini = FALSE );

    /**
     * Returns the pixmap that is associated with this kind of
     * MimeType. The information about this pixmap is taken from
     * the K Comments in the *.kdelnk file.
     * WARNING: This function is NOT reentrant. Copy the returned
     * pixmap before calling the function again.
     */
    virtual QPixmap* getPixmap( const char *_url, bool _mini = FALSE );

    /**
     * The return value may be 0L if there is no comment at all.
     * The comment is taken from the *.kdelnk file '_url.'
     *
     * @returns this file types comment.
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

class ExecutableMimeType : public KMimeType
{
public:
    /**
     * Create an ExecutableMimeType.
     */
    ExecutableMimeType( const char *_mime_type, const char *_pixmap ) :
	KMimeType( _mime_type, _pixmap ) { }

  virtual ~ExecutableMimeType() {};

    virtual bool run( const char *_url );
    
    virtual bool runAsApplication( const char *_url, QStrList *_arguments );
};

class KFMAutoMount : public QObject
{
    Q_OBJECT
public:
    KFMAutoMount( bool _readonly, const char *_format, const char *_device, const char *_mountpoint );
    
public slots:
    void slotFinished( int );
    void slotError( int , const char *, int );
    
protected:
    KIOJob *job;
    QString device;
};

#endif





