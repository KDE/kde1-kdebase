#ifndef kfmw_h
#define kfmw_h

#include <qwidget.h>
#include <qtimer.h>
#include <qstrlist.h>

#include <kiconloader.h>

#define pkfm Kfm::kfm()

class Kfm : public QWidget
{
    Q_OBJECT
public:
  Kfm();
  ~Kfm();
    
  KIconLoader *iconLoader() { return pIconLoader; }
  static Kfm* kfm() { return pKfm; }
  static QStrList* history() { return pHistory; }
  static void addToHistory( const char *_url );
  /**
   * @return false on error.
   */
  static bool saveHTMLHistory( const char *_filename );
  static bool isGoingDown() { return s_bGoingDown; }

    /**
     * Sets and returns is per URL setting enabled - sven
     */
  static bool isURLPropesEnabled () {return bAllowURLProps;}
  static void setURLProps (bool flag) {bAllowURLProps = flag;}
  
    /**
     * Sets and returns does tree-view follows navigation in normal view - david
     */
  static bool isTreeViewFollowMode() {return bTreeViewFollowMode;}
  static void setTreeViewFollowMode(bool flag) { bTreeViewFollowMode = flag;}
  
  /**
   * This function checks if destination is global mime/apps path and
   * if so, makes a local variant of it. It vreates all needed
   * directories and .directory files. It modifies dest so it can be
   * used with kojob or any other functions. If user is a root or has
   * write access to given path nothing is done.
   * This function is here to reduce duplication of code. It is
   * verbatim copy of code from kfmview's slotDropEvent.
   */
  static void setUpDest (QString *dest);

public slots:
  void slotSave();
  void slotShutDown();
  void slotTouch();
  void slotInstallSegfaultHandler();
  
protected:
  KIconLoader *pIconLoader;
  QTimer timer;
  
  static Kfm *pKfm;
  static QStrList *pHistory;
  static bool s_bGoingDown;
  static bool bAllowURLProps; // global option
  static bool bTreeViewFollowMode; // global option
};

#endif
