#ifndef _KRenameWIN_H_
#define _KRenameWIN_H_

#include <qwidget.h>
#include <qdialog.h>
#include <qpushbt.h>
#include <qlabel.h>
#include <qlined.h>
#include <qstring.h>
#include <kurl.h>



class KRenameWin : public QDialog
{
    Q_OBJECT
public:
    
    KRenameWin( QWidget *parent, const char *_src, const char *_dest, bool _modal = FALSE );
    ~KRenameWin();
    
    const char* getNewName() { return lineEdit->text(); }
    
protected:

    QPushButton *b0, *b1, *b2, *b3, *b4;
    QLineEdit *lineEdit;

    QString src;
    QString dest;
    
    bool modal;
    
public slots:
    void b0Pressed();
    void b1Pressed();
    void b2Pressed();
    void b3Pressed();
    void b4Pressed();

signals:
    void result( QWidget* _widget, int _button, const char* _src, const char* _data );
};

#endif

