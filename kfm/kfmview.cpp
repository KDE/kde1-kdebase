#include "kfmview.h"
#include "kbind.h"

#include <qstrlist.h>

KFileView::KFileView( QWidget *parent, const char *name )
    : KHTMLWidget( parent, name, KFileType::getIconPath() )
{
    htmlCache = new HTMLCache();
    connect( htmlCache, SIGNAL( urlLoaded( const char*, const char *) ),
	     this, SLOT( slotImageLoaded( const char*, const char* ) ) );
    connect( this, SIGNAL( imageRequest( const char * ) ), htmlCache, SLOT( slotURLRequest( const char * ) ) );
    connect( this, SIGNAL( cancelImageRequest( const char * ) ),
	     htmlCache, SLOT( slotCancelURLRequest( const char * ) ) );    
}

KFileView::~KFileView()
{
    delete htmlCache;
}

void KFileView::resizeEvent( QResizeEvent *_ev )
{
    parse();
}

void KFileView::dndMouseMoveEvent( QMouseEvent * _mouse )
{
    if ( !pressed )
    {
	KHTMLWidget::dndMouseMoveEvent( _mouse );
	return;
    }
    
    int x = _mouse->pos().x();
    int y = _mouse->pos().y();

    if ( abs( x - press_x ) > Dnd_X_Precision || abs( y - press_y ) > Dnd_Y_Precision )
    {
	QStrList l;
	getSelected( l );

	KPixmap pixmap;

	// Do we drag multiple files ?
	if ( l.count() == 0 )
	    return;
	else if ( l.count() == 1 )
	{
	    KFileType *typ = KFileType::findType( l.first() );
	    pixmap = typ->getPixmap( l.first() );
	}
	else
	{
	    // TODO  Nice icon for multiple files
	    KFileType *typ = KFileType::findType( l.first() );
	    pixmap = typ->getPixmap( l.first() );
	}

	// Put all selected files in one line separated with spaces
	char *s;
	QString tmp = "";
	for ( s = l.first(); s != 0L; s = l.next() )
	{
	    tmp += s;
	    tmp += "\n";
	}
	QString data = tmp.stripWhiteSpace();
	QPoint p = mapToGlobal( _mouse->pos() );
	int dx = - pixmap.width() / 2;
	int dy = - pixmap.height() / 2;

	startDrag( new KDNDIcon( pixmap, p.x() + dx, p.y() + dy ), data.data(), data.length(), DndURL, dx, dy );
    }
}

void KFileView::dndMouseReleaseEvent( QMouseEvent * _mouse )
{
    // Used to prevent dndMouseMoveEvent from initiating a drag before
    // the mouse is pressed again.
    pressed = false;
}

void KFileView::mousePressEvent( QMouseEvent *_mouse )
{
    bool newPainter = FALSE;
    
    if ( clue == 0L )
	return;
    
    HTMLObject *obj;
    
    obj = clue->checkPoint( _mouse->pos().x() + x_offset - LEFT_BORDER, _mouse->pos().y() + y_offset );
    
    if ( painter == NULL )
    {
	printf("mousePressEvent: New Painter\n");
	painter = new QPainter();
	painter->begin( this );
	newPainter = TRUE;
    }
    else
	printf("mousePressEvent: OOps Have Painter\n");
    
    // painter->translate( x_offset, -y_offset );
    
    if ( obj != 0L)
    {
	if (obj->getURL()[0] != 0)
	{
	    // This might be the start of a drag. So we save
	    // the position and set a flag.
	    pressed = true;
	    press_x = _mouse->pos().x();
	    press_y = _mouse->pos().y();    

	    if ( _mouse->button() == LeftButton && obj->isSelected() &&
		 (_mouse->state() & ControlButton) != ControlButton )
	    {
		// Just do nothing. Perhaps we will start a drag here soon...
	    }
	    else if ( _mouse->button() == LeftButton )
	    {
		// Deselect all if the control button is not pressed
		if ( (_mouse->state() & ControlButton) != ControlButton )
		    select( painter, false );
		// Toggle the selected flag
		selectByURL( painter, obj->getURL(), !obj->isSelected() );
	    }
	    else if ( _mouse->button() == RightButton )
	    {
		pressed = FALSE;
		QPoint p = mapToGlobal( _mouse->pos() );
		
		// A popupmenu for a list of URLs or just for one ?
		QStrList list;
		getSelected( list );
		if ( list.count() > 1 )
		{
		    printf("View: list.count() = %i, url = '%s'\n",list.count(),obj->getURL());
		    char*s;
		    for ( s = list.first(); s != 0L; s = list.next() )
			printf(" Entry '%s'\n",s);
		    
		    if ( list.find( obj->getURL() ) != -1 )
			 emit popupMenu2( list, p );
		    else
		    {
			// The selected URL is not marked, so unmark the marked ones.
			select( painter, false );
			emit popupMenu( obj->getURL(), p );
		    }
		}
		else
		    emit popupMenu( obj->getURL(), p );
	    }    	    
	}
	else if ( (_mouse->state() & ControlButton) != ControlButton )
	    select( painter, false );
    }
    else if ( (_mouse->state() & ControlButton) != ControlButton )
	select( painter, false );

    if ( newPainter )
    {
	printf("mousePressEvent: Delete painter\n");
	painter->end();
	delete painter;
	painter = NULL;
    }
}

#include "kfmview.moc"
