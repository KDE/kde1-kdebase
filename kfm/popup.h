// Popup menus for kfm icons. Only the 'New' submenu for the moment.
// (c) David Faure, 1998
#ifndef POPUP_H
#define POPUP_H

#include <qpopmenu.h>
#include <qstrlist.h>

// The 'New' submenu, with 'Folder' and one item per Template
class KNewMenu : public QPopupMenu
{
    Q_OBJECT
public:
    KNewMenu();
    ~KNewMenu() {}

    /**
     * Fills the templates list. Can be called at any time to update it.
     */
    static void fillTemplates();
    
    /**
     * Set the files the popup is shown for
     */
    void setPopupFiles(QStrList & _files) { popupFiles.copy( _files ); }
    void setPopupFiles(const char * _file) {
        popupFiles.clear();
        popupFiles.append( _file );
    }
    
protected slots:

    /**
     * Called when New->* is clicked
     */
    void slotNewFile( int _id );
 
    /**
     * Called before showing the New menu
     */
    void slotCheckUpToDate( );
 
    /*
     * Called when the templates has been copied
     */
    void slotCopyFinished( int id );

private:

    /**
     * Fills the menu from the templates list.
     */
    void fillMenu();

    /**
     * List of all template files. It is important that they are in
     * the same order as the 'New' menu.
     */
    static QStrList * templatesList;

    /**
     * Is increased when templatesList has been updated and
     * menu needs to be re-filled. Menus have their own version and compare it
     * to templatesVersion before showing up
     */
    static int templatesVersion;

    int menuItemsVersion;
    
    /**
     * When the user pressed the right mouse button over an URL a popup menu
     * is displayed. The URL belonging to this popup menu is stored here.
     */
    KStrList popupFiles;

    /*
     * The destination of the copy, for each job being run (job id is the dict key).
     * Used to popup properties for it
     */
    QIntDict <QString> m_sDest;
};

#endif // POPUP_H
