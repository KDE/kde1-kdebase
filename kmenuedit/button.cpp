//
//  kmenuedit
//
//  Copyright (C) 1997 Christoph Neerfeld
//  email:  Christoph.Neerfeld@home.ivm.de or chris@kde.org
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

#include <stdlib.h>

#include <qapp.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qscrbar.h>   // for qDrawArrow
#include <qmsgbox.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
//#include <X11/extensions/shape.h>

#include <kapp.h>
#include <kbutton.h>

#include "button.h"
#include "button.moc"
#include "pmenu.h"

Window debugWin = 0;

extern PMenuItem *global_drop_buffer;

//----------------------------------------------------------------------
//---------------  EditButton  -----------------------------------------
//----------------------------------------------------------------------

EditButton::EditButton( QWidget *parent , const char *name )
  : QButton ( parent, name )
{
  initMetaObject();
  raised = 0;
  connect( this, SIGNAL( pressed() ), this, SLOT( slotPressed () ) );
  connect( this, SIGNAL( released() ), this, SLOT( slotReleased () ) );
  drag = false;
  dndData = 0L; 
  dndIcon = 0L;
}

QSize EditButton::sizeHint()
{
  if( !pixmap() )
    {
      int tw, th;
      QFontMetrics fm = fontMetrics();
      QRect br = fm.boundingRect( text() );
      tw = br.width()  + 6;
      th = br.height() + 6;
      tw += tw/8 + 16;
      th += th/8 + 8;
      return QSize( tw, th );
    }
  return QSize( pixmap()->width()+4, pixmap()->height()+4 ); 
}

void EditButton::setText( const char *t )
{
  btext = t;
  btext.detach();
  if( bpixmap.isNull() )
    {
      QButton::setText(btext);
      return;
    }
  setPixmap( bpixmap );
}

void EditButton::setPixmap( const QPixmap &pixmap )
{
  bpixmap = pixmap;
  int w, h;
  QPainter p;
  QPixmap buf;
  QFontMetrics fm = fontMetrics();
  w = 2 + bpixmap.width() + 4 + fm.width( btext ) + 2 + 10;
  h = ( bpixmap.height() > fm.height() ? bpixmap.height() : fm.height() ) + 4; 
  
  buf.resize( w, h );
  buf.fill( backgroundColor() );
  
  p.begin( &buf );
  if( greyed )
    p.setPen( palette().disabled().text().dark(170) );
  p.drawPixmap( 2, 2, bpixmap );
  p.setFont( font() );
  p.drawText( 2 + bpixmap.width() + 4,        // center text in item
	      0, w, h,
	      AlignVCenter | ShowPrefix | DontClip | SingleLine,
	      btext );
  p.end();

  QButton::setPixmap( buf );
}

void EditButton::slotPressed()
{
  raised = -1;
  repaint();
}

void EditButton::slotReleased()
{
  raised = 0;
  repaint();
}

//--------------------
// drag and drop stuff

void EditButton::startDrag( KDNDIcon *_icon, char *_data, int _size, int _type, int _dx, int _dy )
{
  if ( dndData != 0 )
	free( dndData );
  
  drag = true;
    
  dndData = new char[ _size + 1 ];
  memcpy( dndData, _data, _size );
  dndData[ _size ] = 0;
  
  dndSize = _size + 1;
  dndType = _type;
  dndIcon = _icon;
  dndOffsetX = _dx;
  dndOffsetY = _dy;
  
  dndIcon->raise();
  
  dndLastWindow = (Window)0;
  
  Window root = DefaultRootWindow( kapp->getDisplay() );
  XChangeProperty( kapp->getDisplay(), root, kapp->getDndSelectionAtom(), 
				   XA_STRING, 8,
				   PropModeReplace, (const unsigned char*)dndData, dndSize);
}

Window EditButton::findRootWindow( QPoint & p )
{
  int x,y;
  Window root = DefaultRootWindow( kapp->getDisplay() );
  Window win;
  XTranslateCoordinates( kapp->getDisplay(), root, root, p.x(), p.y(), &x, &y, &win );
  return win;
}

void EditButton::mouseReleaseEvent( QMouseEvent * _mouse )
{
  if ( !drag )
    {
	  dndMouseReleaseEvent( _mouse );
	  return;
    }

  dndIcon->move( -200, -200 );
  
  //printf("************************************************************\n");
  //printf("*** id = %i ****\n", dndIcon->winId());
  //printf("************************************************************\n");
  debugWin = dndIcon->winId();
  
  QPoint p = mapToGlobal( _mouse->pos() );
  
  Window root;
  root = DefaultRootWindow( kapp->getDisplay() );
  
  Window win = findRootWindow( p );
  //printf("************************************************************\n");
  //printf("*** win = %ld **** dndLastWindow = %ld ****\n", win, dndLastWindow );
  //printf("************************************************************\n");
  
  drag = false;
  
  QPoint p2(0,0);
  QPoint p3 = mapToGlobal(p2);
  if( win != findRootWindow(p3) )
    {
      //debug("dropping on another window");
      QString data;
      if( !global_drop_buffer )
	debug("KMenuedit::Program error. global_drop_buffer is not initialised!");
      data += global_drop_buffer->getDirPath() + '/';
      data += global_drop_buffer->getName();
      QFileInfo fi(data);
      if( !fi.exists() || data == "/" )
	{
	  QMessageBox::warning( 0, klocale->translate("KMenuedit"), 
				klocale->translate("Sorry ! You have to save your changes\n"
				"before you can drop this item on "
				"another window.") );
	  delete dndIcon;
	  dndIcon = 0L;
	  dragEndEvent();
	  return;
	}
      data = (QString) "file://" + data;
      if( dndData != 0 )
	delete dndData;
      dndData = new char[ data.length() + 1 ];
      memcpy( dndData, (const char *) data, data.length() );
      dndData[ data.length() ] = 0;
      dndSize = data.length() + 1;
      dndType = DndURL;
      XChangeProperty( kapp->getDisplay(), root, kapp->getDndSelectionAtom(), 
		       XA_STRING, 8,
		       PropModeReplace, (const unsigned char*)dndData, dndSize);
    }
  // If we found a destination for the drop
  if ( win != 0 )
  // if ( dndLastWindow != 0 )
    {	
      //printf("Sending event\n");
	
	  XEvent Event;
	  
	  Event.xclient.type              = ClientMessage;
	  Event.xclient.display           = kapp->getDisplay();
	  Event.xclient.message_type      = kapp->getDndProtocolAtom();
	  Event.xclient.format            = 32;
	  Event.xclient.window            = dndLastWindow;
	  Event.xclient.data.l[0]         = dndType;
	  Event.xclient.data.l[1]         = 0;
	  Event.xclient.data.l[2]         = 0;
	  Event.xclient.data.l[3]         = p.x();
	  Event.xclient.data.l[4]         = p.y();

	  //printf("1\n");
	  XSendEvent( kapp->getDisplay(), dndLastWindow, True, NoEventMask, &Event );	
	  //printf("2\n");
	  XSync( kapp->getDisplay(), FALSE );	
	  //printf("3\n");

	  delete dndData;
	  dndData = 0L;
    }
   else
   {
     //printf("Root Drop Event\n");
       rootDropEvent( p.x(), p.y() );
   }

  delete dndIcon;
  dndIcon = 0L;
   
  dragEndEvent();
}

void EditButton::mouseMoveEvent( QMouseEvent * _mouse )
{
  if ( !drag )
    {
	  dndMouseMoveEvent( _mouse );
	  return;
    }
    
  // dndIcon->hide();
  
  // QPoint p = mapToGlobal( _mouse->pos() );
  QPoint p = QCursor::pos();

  QPoint p2( p.x(), p.y() );
  p2.setX( p2.x() + dndOffsetX );
  p2.setY( p2.y() + dndOffsetY );
  dndIcon->move( p2 );
  // dndIcon->show();
  
}

void EditButton::rootDropEvent( int _x, int _y )
{
  //rootDropEvent();
  //return;  // root drop _DISABLED_

  Window root;
  Window parent;
  Window *children;
  unsigned int cchildren;
    
  //printf("Root Window\n");
  root = DefaultRootWindow( kapp->getDisplay() );
  //printf("Query root tree\n");
  XQueryTree( kapp->getDisplay(), root, &root, &parent, &children, &cchildren );
    
  for ( uint i = 0; i < cchildren; i++ )
  {
      if ( children[i] == debugWin )
	; //printf("******************** root id = %ld *********************\n",children[i] );
      else
      {
	  XEvent Event;
      
	  Event.xclient.type              = ClientMessage;
	  Event.xclient.display           = kapp->getDisplay();
	  Event.xclient.message_type      = kapp->getDndRootProtocolAtom();
	  Event.xclient.format            = 32;
	  Event.xclient.window            = children[ i ];
	  Event.xclient.data.l[0]         = dndType;
	  Event.xclient.data.l[1]         = (long) time( 0L );
	  Event.xclient.data.l[2]         = 0;
	  Event.xclient.data.l[3]         = _x;
	  Event.xclient.data.l[4]         = _y;
	  XSendEvent( kapp->getDisplay(), children[ i ], True, NoEventMask, &Event);
	  XSync( kapp->getDisplay(), FALSE );	
      }
  }
    
  //printf("Done\n");
  
  // Clean up.
  rootDropEvent();
}

void EditButton::rootDropEvent()
{
  if ( dndIcon != 0L )
	delete dndIcon;
  dndIcon = 0L;
  
  if ( dndData != 0L )
	delete dndData;
  dndData = 0L;
}

EditButton::~EditButton()
{
  if ( dndData != 0L )
	free( dndData );
}
