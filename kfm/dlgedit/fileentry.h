#ifndef fileentry_h
#define fileentry_h

#include <qlined.h>
#include <qstring.h>
#include <qstrlist.h>

class KFileEntry : public QLineEdit
{
    Q_OBJECT
public:
    KFileEntry( QWidget *_parent, const char *_dir );
    ~KFileEntry();

protected:
    virtual void keyPressEvent( QKeyEvent *_ev );

    QString directory;

    int possibility;
    QStrList possibilityList;
    QString guess;
    QString dir2;
};

#endif
