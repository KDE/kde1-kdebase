#ifndef KDROPSITE_H
#define KDROPSITE_H

#include <qwidget.h>
#include <qdropsite.h>
#include <qcursor.h>
#include <qbitmap.h>


class KDropSite: public QObject, QDropSite
{
    Q_OBJECT
public:
    KDropSite( QWidget * parent = 0);

signals:
    void dropAction(QDropEvent*);
    void dragLeave(QDragLeaveEvent*);
    void dragEnter(QDragEnterEvent*);
    void dragMove(QDragMoveEvent*);

protected:
    void dragEnterEvent( QDragEnterEvent * );
    void dragMoveEvent( QDragMoveEvent * );
    void dragLeaveEvent( QDragLeaveEvent * );
    void dropEvent( QDropEvent * );
};


#endif
