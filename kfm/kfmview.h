#ifndef KFMVIEW_H
#define KFMVIEW_H

class HTMLToolTip;
class KFileView;

#include <qstrlist.h>
#include <qtooltip.h>

#include "kbind.h"
#include "htmlcache.h"

#include <html.h>

class KFileView : public KHTMLWidget
{
    Q_OBJECT
public:
    KFileView( QWidget *parent=0, const char *name=0 );
    virtual ~KFileView();
    
signals:
    /*********************************************************
     * The user clicked the right mouse button on the global
     * position _p over an URL while other URLs are selected, too.
     * In this case a list of all selected URLs is used as a parameter
     * to this signal.
     */
    void popupMenu2( QStrList &_urls, const QPoint &_p );

protected:
    virtual void mousePressEvent( QMouseEvent * );
    virtual void dndMouseReleaseEvent( QMouseEvent * _mouse );
    virtual void dndMouseMoveEvent( QMouseEvent * );

    HTMLCache *htmlCache;
};

#endif
