#ifndef kfm_h
#define kfm_h

#include <qwidget.h>
#include <kiconloader.h>

#define pkfm KFM::kfm()

class KFM : public QWidget
{
    Q_OBJECT
public:
    KFM();
    ~KFM();
    
    KIconLoader *iconLoader() { return pIconLoader; }
    static KFM* kfm() { return pKfm; }
    
public slots:
    void slotSave();

protected:
    KIconLoader *pIconLoader;
    static KFM *pKfm;
};

#endif
