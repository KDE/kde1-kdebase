#ifndef KBIND_H
#define KBIND_H

#include <qstring.h>
#include <qlist.h>
#include <qstrlist.h>
#include <qfile.h>

#include <kurl.h>
#include <kapp.h>
#include <Kconfig.h>

#include "kioserver.h"
#include "kpixmap.h"

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

class KFileType;

/// The application bindings.
/**
  For every KFileType you can create multiple bindings. These binding
  contain the name of the program, the command to start it and
  the protocols supported.
  */
class KFileBind
{
public:
    /*********************************************************
     * Creates a FileBinding with the name as 1st parameter. The 2nd
     * parameter is the command line. You may use '%f' as a wildcard for the path
     * (makes only sense for the protocol 'file') or '%u' for the URL of a file.
     * The last 3 parameters conatain one or more supported protocols. A Web Browser
     * will for example support 'file', 'http', 'ftp' while a graphics program will
     * only support 'file'. While the WWW Browser will use '%u', the graphics program
     * will use '%f'.
     */
    KFileBind( const char *_name, const char *_cmd, const char *_prot1,
	       const char *_prot2 = 0, const char * _prot3 = 0,
	       const char* _prot4 = 0, const char * _prot5 = 0 );

    /*********************************************************
     * Returns the programs name
     */
    const char* getProgram() { return (const char*)program; }
    /*********************************************************
     * Returns the command string.
     */
    const char* getCmd() { return (const char*)cmd; }

    /*********************************************************
     * Tests wether this file binding supports the specified protocol.
     */
    bool supportsProtocol( const char *_protocol );

    /// Register another application
    /**
      Every application is registered using this function. This way we have
      a list of all registered applications.
      */
    static void appendApplication( const char *_appname ) { appList.append( _appname ); }
    
    /// Returns the name of the first registered application
    static const char* getFirstApplication() { return appList.first(); }
    /// Returns the name of the next registered application
    /**
      Use this function only after a call to 'getFirstApplication'.
      */
    static const char* getNextApplication() { return appList.next(); }
    /// Clear the list of registered applications.
    /**
      This function is used if the user changes some of the application/filetype
      entries and they have to be refreshed.
      */
    static void clearApplicationList() { appList.clear(); }
    
protected:
    /*********************************************************
     * The programs name.
     */
    QString program;
    /*********************************************************
     * The command string.
     */
    QString cmd;
    /*********************************************************
     * First supported protocol. May not be null.
     */
    QString protocol1;
    /*********************************************************
     * Second supported protocol. May be null.
     */
    QString protocol2;
    /*********************************************************
     * Third supported protocol. May be null.
     */
    QString protocol3;
    QString protocol4;
    QString protocol5;

    static QStrList appList;
};

/// The file patterns
/**
  For every file extension ( for example ".html" )
  you can create a KFileType. This way the program knows which
  icon to use and which programs can handle the data.
  Have a look at KFileBind. Multiple extensions can be organized
  in KFileType ( for example *.tgz, *.tar.gz ) since they
  mean the same document class.
 */
class KFileType
{
public:
    /*********************************************************
     * Create a KFileType. You must later on call 'setPixmap' and
     * 'setPattern'.
     */
    KFileType() { }
    /*********************************************************
     * Create a KFileType with pattern and icon.
     */
    KFileType( const char *_pattern, const char *_pixmap );

    /*********************************************************
     * Add a binding to this type.
     */
    virtual void append( KFileBind * );
    /*********************************************************
     * Set the icon.
     */
    virtual void setPixmap( const char * );

    /// Add a pattern which matches this type.
    virtual void addPattern( const char *_p ) { if ( pattern.find( _p ) == -1 ) pattern.append( _p ); }

    /// Set the default bindings name
    virtual void setDefaultBinding( const char *_b ) { defaultBinding = _b; defaultBinding.detach(); }
    /*********************************************************
     * Returns the extensions assoziated with this FileType, for
     * example '.html' or '.cpp'.
     */
    virtual QStrList& getPattern() { return pattern; }

    /// Returns the name of this binding
    /**
      The file type in the file '$KDEDIR/filetypes/Spreadsheet.kdelnk' for
      example is called 'Spreadsheet'.
      */
    const char* getName() { return name.data(); }

    /// Returns this file types comment.
    /**
      The return vakue may be 0L if there is no comment at all.
      This method does not use _url but some overloading methods do so.
      */
    virtual QString getComment( const char *_url ) { return QString( comment.data() ); }

    /// Sets the comment
    void setComment( const char *_c) { comment = _c; }

    /// Marks this file type to be only an application pattern
    /**
      Since it is possible to give binaries an icon, we must create a file type
      for those binaries. This functions sets a flag that indicates that this is not
      a real file type.
      */
    void setApplicationPattern() { bApplicationPattern = TRUE; }
    /// Tells wether this file type is only an application pattern
    /**
      See #setApplicationPattern' for details.
      */
    bool isApplicationPattern() { return bApplicationPattern; }
    
    /**
      Returns the full qualified filename ( not an URL ) for the icons
      pixmap.
     */
    virtual const char* getPixmapFile( const char *_url );
    /*********************************************************
     * Returns the pixmap that is associated with this kind of
     * FileType.
     */
    virtual QPixmap& getPixmap( const char *_url );
    /// Return a pointer to the default binding.
    /**
      The return may be 0. In this case the user did not specify
      a special default binding. That does not mean that there is no
      binding at all.
      */
    virtual const char* getDefaultBinding() { return defaultBinding; }
    
    /*********************************************************
     * Tells wether bindings are available.
     */
    virtual bool hasBindings() { return !bindings.isEmpty(); }
    /*********************************************************
     * Gets the first binding if available. Dont use 'firstBinding' and
     * 'nextBinding' in two 'geschachtelten' loops at once.
     */
    virtual KFileBind* firstBinding() { return bindings.first(); }
    /*********************************************************
     * Gets the next binding if available. Dont use 'firstBinding' and
     * 'nextBinding' in two 'geschachtelten' loops at once.
     * Call 'firstBinding' before calling this function.
     */
    virtual KFileBind* nextBinding() { return bindings.next(); }

    /*********************************************************
     * Tries to find a FileType for the file _url. If no special
     * FileType is found, the default FileType is returned.
     */
    static KFileType* findType( const char *_url );

    /// Find type by pattern.
    /**
      If for the given pattern no KFileType has been created, the function
      will retun 0L;
      */
    static KFileType *findByPattern( const char *_pattern );
    
    /// Finds a file type by its name
    /**
      The file type in the file '$KDEDIR/filetypes/Spreadsheet.kdelnk' for
      example is called 'Spreadsheet'.
      */
    static KFileType *findByName( const char *_name );
    
    /*********************************************************
     * Fills _list with all bindings for _url. Each binding is 
     * represented by a string. You may use such a string as text
     * of an item in a popup menu.
     */
    static void getBindings( QStrList &_list, const char *_url, bool _isdir );
    /// Tries to execute the binding named _binding for the file _url.
    /**
      The default binding is "Open". For example when the user double clicks
      on something and you dont know a specific binding, then select "Open".
      If you want to pass some URLs to an executable ( for examples documents to open )
      then you can pass a list of all URLs in '_args'. This makes sense only with 
      executables or *.kdelnk files of the type "Exec".
      '_url' may be an executable, a directory, or just a document.
      If it is a directory or a document then the '_args' dont matter. If in the other case
      '_url' is an executable then '_binding' is not used.
      */
    static void runBinding( const char *_url, const char *_binding, QStrList * _args = 0L );
    /// Run the default binding
    /**
      If there is only one binding, this one is executed otherwise the default
      binding is executed. If there are multiple bindings and none of them is the default
      one, the first one is executed. Usually used when the user double clicks
      on an URL. Files named *.kdelnk are executed, too.
      This function only determines the default binding and calls 'runBinding( .., .. )'
      to do the real job. If no binding can be found, the binding "Open" is given a try.
      */
    static void runBinding( const char *_url );

    /// Open a KConfig file
    /**
      Returns a KConfig if the file starts with
      "[KDE Desktop Entry]"
      This group will be already selected. Otherwise 0L is returned.
      */
    static KFMConfig* openKFMConfig( const char *_url );

    /// Returns the first file type
    /**
      Returns 0L if there is no file type at all.
      */
    static KFileType *getFirstFileType();
    /// Returns the next file type
    /**
      Use this function only after a call to 'getFirst'.
      Returns 0L if there are no more file types.
      */
    static KFileType *getNextFileType();

    /// This function deletes all file bindings and types.
    /**
      This is done if the user changed some application/filetype settings.
      */
    static void clearAll();
    
    /*********************************************************
     * Call this function before you call any other function. This
     * function initializes some global variables.
     */
    static void init();
    
    /// Scan the $KDEDIR/apps directory for application bindings
    static void initApplications( const char *_path );

    /// Scan the $KDEDIR/filetypes directory for the file types
    static void initFileTypes( const char *_path );
    
    /// Return the path for the icons
    static const char* getIconPath() { return icon_path; }
    static const char* getDefaultPixmap() { return defaultPixmap; }    
    static const char* getExecutablePixmap() { return executablePixmap; }    
    static const char* getBatchPixmap() { return batchPixmap; }    
    static const char* getFolderPixmap() { return folderPixmap; }    

protected:
    /*********************************************************
     * List with all bindings for this type.
     */
    QList<KFileBind> bindings;
    /*********************************************************
     * The full qualified filename ( not an URL ) of the icons
     * pixmap.
     */
    QString pixmap_file;
    /*********************************************************
     * The pattern matching this file. For example: ".html".
     */
    QStrList pattern;
    /*********************************************************
     * The pixmap used for the icon.
     */
    QPixmap pixmap;

    /// Holds the default binding
    /**
      This string may be 0. In this case the user did not specify
      a special default binding. That does not mean that there is no
      binding at all. Attention: Perhaps a binding with that name does
      not exist for a strange reason.
      */
    QString defaultBinding;
    
    /// Holds the name of the file type
    /**
      The file type in the file '$KDEDIR/filetypes/Spreadsheet.kdelnk' for
      example is called 'Spreadsheet'.
      */
    QString name;

    /// Holds a comment to this file type.
    QString comment;

    /// The path to the icons
    static char icon_path[ 1024 ];
    /// Default pixmap for executables
    static char executablePixmap[ 1024 ];
    /// Default pixmap for batch files
    static char batchPixmap[ 1024 ];
    /// General default pixmap
    static char defaultPixmap[ 1024 ];
    /// Default pixmap for folders
    static char folderPixmap[ 1024 ];    

    /// This flag is set if this file type is only an application pattern
    /**
      See 'setApplicationPattern' for details.
      */
    bool bApplicationPattern;
};

/// A special KFileType for folders
/**
  Looks in the folder for a file called '.desktop'. This file may include
  an icon entry. Otherwise the default icon is used.
  */
class KFolderType : public KFileType
{
public:
    /// Constructor
    KFolderType() : KFileType() { }

    /// Get the pixmap files full name
    /** WARNING: This function is NOT reentrant. Copy the returned
      string before calling the function again.
      */
    virtual const char* getPixmapFile( const char *_url );

    /// Get the pixmap for the given URL.
    /** Returns the pixmap that is associated with this kind of
      FileType. The information about this pixmap is taken from
      the K Comments in the *.kdelnk file.
      WARNING: This function is NOT reentrant. Copy the returned
      pixmap before calling the function again.
      */
    virtual QPixmap& getPixmap( const char *_url );

    virtual QString getComment( const char *_url );
    
protected:

    /// The pixmaps full filename
    /** When a directory contains a '.directory' file, then the corresponding
      icon filename is stored here. Since another call to getPixmap may overwrite
      this variable, this causes the whole stuff to be NOT reentrant.
      */
    QString pixmapFile2;
};


/// This class is used for all files with the ending *.kdelnk
/**
  The *.kdelnk files may include an icon entry.
  */
class KDELnkFileType : public KFileType
{
public:
    /*********************************************************
     * Create a KDELnkFileType. You need not specify pixmaps or
     * patterns. All information is in the *.kdelnk files.
     */
    KDELnkFileType() : KFileType() { }

    /*********************************************************
     * WARNING: This function is NOT reentrant. Copy the returned
     * string before calling the function again.
     */
    virtual const char* getPixmapFile( const char *_url );

    /*********************************************************
     * Returns the pixmap that is associated with this kind of
     * FileType. The information about this pixmap is taken from
     * the K Comments in the *.kdelnk file.
     * WARNING: This function is NOT reentrant. Copy the returned
     * pixmap before calling the function again.
     */
    virtual QPixmap& getPixmap( const char *_url );

    /// Returns this file types comment.
    /**
      The return value may be 0L if there is no comment at all.
      The comment is taken from the *.kdelnk file '_url.'
      */
    virtual QString getComment( const char *_url );
};

#endif
