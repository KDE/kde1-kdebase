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
};

#endif
