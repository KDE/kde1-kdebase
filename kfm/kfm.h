#ifndef kfm_h
#define kfm_h

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
