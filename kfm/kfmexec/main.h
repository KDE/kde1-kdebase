#ifndef _main_h
#define _main_h

#include <qobject.h>
#include <qstring.h>
#include <qstrlist.h>

class KFMExec : public QObject
{
    Q_OBJECT
public:
    KFMExec( int argc, char **argv );
    
public slots:
    void slotFinished();
    
protected:
    int counter;
    int expectedCounter;
    QString command;
    QString files;
    QStrList fileList;
    QStrList urlList;
};

#endif
