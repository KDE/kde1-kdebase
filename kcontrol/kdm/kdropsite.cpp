
#include "kdropsite.moc"
#include <kapp.h>
#include <kiconloader.h>
#include <qevent.h>
#include <qpixmap.h>
#include <qdragobject.h>
#include <qimage.h>


KDropSite::KDropSite( QWidget * parent ) : QObject( parent ), QDropSite( parent )
{
  //debug("KDropSite constructor");
}

void KDropSite::dragMoveEvent( QDragMoveEvent *e )
{
  //debug("dragMove");
  emit dragMove(e);
}

void KDropSite::dragEnterEvent( QDragEnterEvent *e )
{
  //debug("dragEnter");
  emit dragEnter(e);
}

void KDropSite::dragLeaveEvent( QDragLeaveEvent *e )
{
  //debug("dragLeave");
  emit dragLeave(e);
}

void KDropSite::dropEvent( QDropEvent *e )
{
  //debug("drop");
  emit dropAction(e);
}


