#ifndef kfm_h
#define kfm_h

#include <qwidget.h>

class KFM : public QWidget
{
    Q_OBJECT
public:
    KFM();
    ~KFM();
    
public slots:
    void slotSave();
    
};

#endif
