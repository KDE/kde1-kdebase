/*
  This file holds the definitions for all classes used to
  display a properties dialog.
  */

#ifndef KFMPROPS_H
#define KFMPROPS_H

#include <sys/stat.h>
#include <unistd.h>

#include <qtabdlg.h>
#include <qbttngrp.h>
#include <qradiobt.h>
#include <qlabel.h>
#include <qlayout.h>
#include <stdio.h>
#include <qstring.h>
#include <qchkbox.h>
#include <qlined.h>
#include <qlist.h>
#include <qcombo.h>
#include <qgrpbox.h>
#include <qpushbt.h>
#include <qlistbox.h>
#include <qtooltip.h>

#include <kurl.h>
#include <kiconloaderdialog.h>
#include <kiconloader.h>

class Properties;

/// A Page in the Properties dialog
/**
  This is an abstract class. You must inherit from this class
  to build a new kind of page.
  */
class PropsPage : public QWidget
{
    Q_OBJECT
public:
    /// Constructor
    PropsPage( Properties *_props );

    /// Returns the name that should appear in the tab.
    virtual const char* getTabName() { return ""; }
    /// Apply all changes to the file.
    /**
      This function is called when the user presses 'Ok'. The last page inserted
      is called first.
      */
    virtual void applyChanges() { }
    
protected:
    /// Pointer to the dialog
    Properties *properties;

    int fontHeight;
};

/// 'General' page
/**
  This page displays the name of the file, its size and access times.
  */
class FilePropsPage : public PropsPage
{
    Q_OBJECT
public:
    /// Constructor
    FilePropsPage( Properties *_props );

    virtual const char* getTabName() { return klocale->translate("&General"); }
    /// Applies all changes made
    /** 'General' must be always the first page in the dialog, since this
      function may rename the file which may confuse other applyChanges
      functions. When this page is the first one this means that this applyChanges
      function is the last one called.
      */
    virtual void applyChanges();
    
    /// Tests wether the file specified by _kurl needs a 'General' page.
    static bool supports( KURL *_kurl );
    
protected:
    QLineEdit *name;
    QLineEdit *lname;

    QBoxLayout *layout;		// BL: layout mngt

    /// The initial filename
    QString oldName;
};

/// 'Permissions' page
/**
  In this page you can modify permissions and change
  the owner of a file.
  */
class FilePermissionsPropsPage : public PropsPage
{
    Q_OBJECT
public:
    /// Constructor
    FilePermissionsPropsPage( Properties *_props );

    virtual const char* getTabName() { return klocale->translate("&Permissions"); }
    virtual void applyChanges();

    /// Tests wether the file specified by _kurl needs a 'Permissions' page.
    static bool supports( KURL *_kurl );
    
protected:
    QCheckBox *permBox[3][4];
    QComboBox *grp;
    QLineEdit *owner;

    /// Old permissions
    mode_t permissions;
    /// Old group
    QString strGroup;
    /// Old owner
    QString strOwner;

    /// Changeable Permissions
    static mode_t fperm[3][4];
};

/// The main class.
/**
  This one is visible to the one who created the dialog.
  It brings up a QTabDialog.
  This class must be created with (void)new Properties(...)
  It will take care of deleting itself.
  */
class Properties : public QObject
{
    Q_OBJECT
public:
    /// Bring up a dialog for the given URL.
    Properties( const char *_url );
    ~Properties();

    /// Returns a parsed URL
    KURL* getKURL() { return kurl; }
    /// Returns the URL text
    const char* getURL() { return url.data(); }
    /// Returns a pointer to the dialog
    QTabDialog* getTab() { return tab; }

    /// Usually called from a PropsPage in order of emitting the signal.
    void emitPropertiesChanged( const char *_new_name );
    
public slots:
    /// Called when the user presses 'Ok'.
    void slotApply();      // Deletes the Properties instance
    void slotCancel();     // Deletes the Properties instance

signals:
    /// Notify about changes in properties and rnameings.
   /**
     For example the root widget needs to be informed about renameing. Otherwise
     it would consider the renamed icon a new icon and would move it to the upper left
     corner or something like that.
     In most cases you wont need this signal because KIOManager is informed about changes.
     This causes KFileWindow for example to reload its contents if necessary.
     If the name did not change, _name is 0L.
     */
    void propertiesChanged( const char *_url, const char *_new_name );
    void propertiesCancel();
    
protected:
    /// Inserts all pages in the dialog.
    void insertPages();

    /// The URL of the file
    QString url;
    /// The parsed URL
    KURL *kurl;
    /// List of all pages inserted ( first one first )
    QList<PropsPage> pageList;
    /// The dialog
    QTabDialog *tab;
};

/// Edit "KDE Desktop Entry" Files.
/**
  Used to edit the files containing
  [KDE Desktop Entry]
  Type=Application

  Such files are used to represent a program in kpanel and kfm.
  */
class ExecPropsPage : public PropsPage
{
    Q_OBJECT
public:
    /// Constructor
    ExecPropsPage( Properties *_props );

    virtual const char* getTabName() { return klocale->translate("E&xecute"); }
    virtual void applyChanges();

    static bool supports( KURL *_kurl );

public slots:
    void slotBrowseExec();
    
private slots:
    void enableCheckedEdit();
    
protected:
    
    QLineEdit *execEdit;
//    QLineEdit *pathEdit; not used
    KIconLoaderButton *iconBox;
    QCheckBox *terminalCheck;
    QLineEdit *terminalEdit;
    QLineEdit *swallowExecEdit;
    QLineEdit *swallowTitleEdit;
    QButton *execBrowse;

    QString execStr;
//    QString pathStr; not used
    QString iconStr;
    QString swallowExecStr;
    QString swallowTitleStr;
    QString termStr;
    QString termOptionsStr;
};

/// Edit "KDE Desktop Entry" Files.
/**
  Used to edit the files containing
  [KDE Desktop Entry]
  URL=....

  Such files are used to represent a program in kpanel and kfm.
  */
class URLPropsPage : public PropsPage
{
    Q_OBJECT
public:
    /// Constructor
    URLPropsPage( Properties *_props );

    virtual const char* getTabName() { return klocale->translate("U&RL"); }
    virtual void applyChanges();

    static bool supports( KURL *_kurl );

protected:
    QLineEdit *URLEdit;
    KIconLoaderButton *iconBox;

    QString URLStr;
    QString iconStr;

    QPixmap pixmap;
    QString pixmapFile;
};

class DirPropsPage : public PropsPage
{
    Q_OBJECT
public:
    /// Constructor
    DirPropsPage( Properties *_props );
    ~DirPropsPage() {};

    virtual const char* getTabName() { return klocale->translate("&Dir"); }
    virtual void applyChanges();

    static bool supports( KURL *_kurl );

public slots:
    void slotWallPaperChanged( int );
    void slotBrowse(); 
    void slotApply();
    void slotApplyGlobal();
    
protected:
    void showSettings(QString filename);
    void drawWallPaper();
    virtual void paintEvent ( QPaintEvent *);
    virtual void resizeEvent ( QResizeEvent *);
    
    QPushButton *applyButton;
    QPushButton *globalButton;
    QPushButton *browseButton;
    
    KIconLoaderButton *iconBox;
    QComboBox *wallBox;

    QString wallStr;
    QString iconStr;

    QPixmap wallPixmap;
    QString wallFile;
    int imageX, imageW, imageH, imageY;
};

/// Edit "KDE Desktop Entry" Files.
/**
  Used to edit the files containing
  [KDE Desktop Entry]
  Type=Application

  Such files are used to represent a program in kpanel and kfm.
  */
class ApplicationPropsPage : public PropsPage
{
    Q_OBJECT
public:
    /// Constructor
    ApplicationPropsPage( Properties *_props );

    virtual const char* getTabName() { return klocale->translate("&Application"); }
    virtual void applyChanges();

    static bool supports( KURL *_kurl );

public slots:
    void slotDelExtension();
    void slotAddExtension();    

protected:

    void addMimeType( const char * name );

    QLineEdit *binaryPatternEdit;
    QLineEdit *commentEdit;
    QLineEdit *nameEdit;
    QListBox  *extensionsList;
    QListBox  *availableExtensionsList;
    QPushButton *addExtensionButton;
    QPushButton *delExtensionButton;
    
    QBoxLayout *layout, *layoutH, *layoutV;

    QString nameStr;
    QString extensionsStr;
    QString binaryPatternStr;
    QString commentStr;
};

/// Edit "KDE Desktop Entry" Files.
/**
  Used to edit the files containing
  [KDE Desktop Entry]
  Type=FileType

  Such files are used to represent a program in kpanel and kfm.
  */
class BindingPropsPage : public PropsPage
{
    Q_OBJECT
public:
    /// Constructor
    BindingPropsPage( Properties *_props );

    virtual const char* getTabName() { return klocale->translate("&Binding"); }
    virtual void applyChanges();

    static bool supports( KURL *_kurl );

protected:
    
    QLineEdit *commentEdit;
    QLineEdit *patternEdit;
    QLineEdit *mimeEdit;
    KIconLoaderButton *iconBox;
    QComboBox *appBox;
    QStrList kdelnklist; // holds the kdelnk names for the combobox items

    QPixmap pixmap;
    QString pixmapFile;
};

class DevicePropsPage : public PropsPage
{
    Q_OBJECT
public:
    DevicePropsPage( Properties *_props );
    ~DevicePropsPage() { }

    virtual const char* getTabName() { return klocale->translate("De&vice"); }
    virtual void applyChanges();

    static bool supports( KURL *_kurl );
    
protected:
    QLineEdit* device;
    QLineEdit* mountpoint;
    QCheckBox* readonly;
    QLineEdit* fstype;
    KIconLoaderButton* mounted;
    KIconLoaderButton* unmounted;

    QPixmap pixmap;
    QString pixmapFile;

    QString deviceStr;
    QString mountPointStr;
    QString mountedStr;
    QString unmountedStr;
    QString readonlyStr;
    QString fstypeStr;
};

#endif

