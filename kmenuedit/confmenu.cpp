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
#include <qfont.h>
#include <qlist.h>
#include <qcombo.h>
#include <qlabel.h>
#include <qlined.h>
#include <qscrbar.h>
#include <qmsgbox.h>
#include <qchkbox.h>
#include <qapp.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qdrawutl.h>
#include <qcursor.h>
#include <qkeycode.h>
#include <qstrlist.h>
#include <qlistbox.h>
#include <qobjcoll.h>
#include <qpushbt.h>

#include <kiconloader.h>
#include <kstatusbar.h>
#include <drag.h>

#include "pmenu.h"
#include "confmenu.h"
#include "confmenu.moc"
#include "entrydialog.h"
#include "kmenuedit.h"

extern KIconLoader *global_pix_loader;
const short glob_but_height = 24;
PMenuItem *global_pmenu_buffer = NULL;
PMenuItem *global_drop_buffer = NULL;
extern KStatusBar *global_status_bar;
extern bool changes_to_save;

static bool isKdelnkFile(const char* name){
  QFile file(name);
  if (file.open(IO_ReadOnly)){
    char s[19];
    int r = file.readLine(s, 18);
    if(r > -1){
      s[r] = '\0';
      file.close();
      return (QString(s).left(17) == "# KDE Config File");
    }
    file.close();
  }
  return FALSE;
}

//----------------------------------------------------------------------
//---------------  MENUBUTTON  -----------------------------------------
//----------------------------------------------------------------------

static MoveMode move_mode = MoveNone;

MenuButton::MenuButton( PMenuItem *p_it, int i, PMenu *p_parent, QWidget *parent,
                          const char *name )
  : EditButton( parent, name )
{
  initMetaObject();
  pmenu_item = p_it;
  pmenu_parent = p_parent;
  setGreyed(pmenu_item->isReadOnly());
  id = i;
  type = pmenu_item->getType();
  if( type == separator )
    setText( klocale->translate("Separator") );
  else
    setText( pmenu_item->getText() );
  setPixmap( pmenu_item->getPixmap() );
  dialog = NULL;
  dialog_open  = FALSE;
  move_button  = FALSE;
  move_group   = FALSE;
  submenu_open = FALSE;
  left_pressed = FALSE;
  if( type == submenu )
    {
      popmenu.insertItem(klocale->translate("Open/ Close"),
			   this, SLOT(open()));
      connect( this, SIGNAL(clicked(int)), this, SLOT(sOpen(int)) );
    }
  if( (pmenu_item->isReadOnly()) )
    {
      popmenu.insertItem(klocale->translate("Copy"), this, SLOT(copyItem()));
      popmenu.insertItem(klocale->translate("View"), this, SLOT(change_item()));
      //popmenu.insertItem(klocale->translate("! PROTECTED Button !"));
    }
  else
    {
      popmenu.insertItem(klocale->translate("Change"), this, SLOT(change_item()));
      popmenu.insertSeparator();
      popmenu.insertItem(klocale->translate("Select item for moving"), this, SLOT(move_item()));
      popmenu.insertItem(klocale->translate("Select menu for moving"), this, SLOT(move_menu()));
      popmenu.insertSeparator();
      popmenu.insertItem(klocale->translate("New"), this, SLOT(new_item()));
      popmenu.insertItem(klocale->translate("Cut"), this, SLOT(cutItem()));
      popmenu.insertItem(klocale->translate("Copy"), this, SLOT(copyItem()));
      popmenu.insertItem(klocale->translate("Paste"), this, SLOT(pasteItem()));
      popmenu.insertSeparator();
      popmenu.insertItem(klocale->translate("Delete"), this, SLOT(delete_item()));
    }
  connect( this, SIGNAL(Rpressed(int)), this, SLOT(popupMenu(int)) );
  setFocusPolicy(StrongFocus);
}

MenuButton::~MenuButton()
{
  if( dialog )
    delete dialog;
}

void MenuButton::copyItem()
{
  if( global_pmenu_buffer )
    {
      delete global_pmenu_buffer;
      global_pmenu_buffer = NULL;
    }
  global_pmenu_buffer = new PMenuItem( *pmenu_item );
  changes_to_save = TRUE;
  ((KMenuEdit *) KApplication::getKApplication()->mainWidget())->setUnsavedData(TRUE);
}

void MenuButton::open()
{
  if( dialog_open )
    return;
  QPoint p = pos() + parentWidget()->parentWidget()->pos();
  QPoint p2(width()-2, 6);
  p += p2;
  pmenu_item->getMenu()->popupConfig(p, parentWidget()->parentWidget()->parentWidget() );
  if( submenu_open )
    {
      submenu_open = FALSE;
      disconnect( pmenu_item->getMenu(), SIGNAL(reposition()),
		  this, SLOT(childRepos()) );
    }
  else
    {
      submenu_open = TRUE;
      connect( pmenu_item->getMenu(), SIGNAL(reposition()),
	       this, SLOT(childRepos()) );
    }
  repaint();
}

void MenuButton::delete_item()
{
  QMessageBox msg_box;
  if( pmenu_item->getType() == submenu )
    {
      if( !msg_box.query( klocale->translate("Warning !"),
			  klocale->translate("If you delete this button you will lose all of its submenus"),
			  klocale->translate("OK"),
			  klocale->translate("Cancel") ) )
	{ return; }
    }
  emit delButton( id );
  changes_to_save = TRUE;
  ((KMenuEdit *) KApplication::getKApplication()->mainWidget())->setUnsavedData(TRUE);
}

void MenuButton::move_item()
{
    QApplication::setOverrideCursor(CrossCursor, TRUE); 
    move_mode = MoveItem;
}

void MenuButton::move_menu()
{
    QApplication::setOverrideCursor(CrossCursor, TRUE); 
    move_mode = MoveMenu;
}

void MenuButton::change_item()
{
  if( dialog_open )
    return;
  EntryType type = pmenu_item->getType();
  dialog = new EntryDialog(this);
  dialog_open = TRUE;
  dialog->c_type->setCurrentItem( type - 1 );
  dialog->typeActivated( type - 1 );
  dialog->i_name->setText(pmenu_item->getText());
  dialog->i_fname->setText(pmenu_item->getName());
  dialog->i_pixmap->setText(pmenu_item->getPixmapName());
  dialog->i_comment->setText(pmenu_item->getComment());
  dialog->b_pixmap->setPixmap( pmenu_item->getPixmap() );
  QString temp_name = pmenu_item->getBigPixmapName();
  dialog->b_big_pixmap->setPixmap( pmenu_item->getBigPixmap());
  dialog->i_big_pixmap->setText(temp_name);
  if( type == unix_com || type == swallow_com )
    {
      if( type == unix_com )
	{
	  dialog->i_term_opt->setText(pmenu_item->getTermOpt());
	}
      else
	{
	  dialog->i_swallow_exec->setText(pmenu_item->getSCommand());
	  dialog->i_swallow_title->setText(pmenu_item->getSTitle());
	  dialog->i_sexec->setText(pmenu_item->getCommand());
	}
      dialog->i_command->setText(pmenu_item->getCommand());
      dialog->i_dir->setText(pmenu_item->getExecDir());
      dialog->i_pattern->setText(pmenu_item->getPattern());
      QString prot = pmenu_item->getProtocols();
      dialog->cb_file->setChecked(prot.contains("file"));
      dialog->cb_ftp->setChecked(prot.contains("ftp"));
      dialog->cb_http->setChecked(prot.contains("http"));
      dialog->cb_tar->setChecked(prot.contains("tar"));
      dialog->cb_info->setChecked(prot.contains("info"));
      dialog->cb_man->setChecked(prot.contains("man"));
      dialog->ch_use_term->setChecked(pmenu_item->useTerm());
      QString str_list, value;
      str_list = pmenu_item->getExtensions();
      dialog->l_inside->clear();
      if(!str_list.isEmpty())
	{
	  int i, len = str_list.length();
	  for( i = 0; i < len; i++ )
	    {
	      if( str_list[i] != ';' )
		{
		  value += str_list[i];
		  continue;
		}
	      dialog->l_inside->inSort(value);
	      value.truncate(0);
	    }
	}
    }
  else if( type == url )
    dialog->i_url->setText(pmenu_item->getUrl());
  else if( type == device )
    {
      dialog->i_device->setText(pmenu_item->getDevice());
      dialog->i_mount->setText(pmenu_item->getMountP());
      dialog->i_fstype->setText(pmenu_item->getFSType());
      QString temp_name = pmenu_item->getUMPixmapName();
      if( !temp_name.isEmpty() )
	dialog->b_mount_pix->setPixmap( global_pix_loader->loadApplicationIcon(temp_name, 70, 70));
      dialog->i_mount_pix->setText(temp_name);
      dialog->cb_read_only->setChecked(pmenu_item->isDevReadOnly());
    }
  if( pmenu_item->isReadOnly() )
    {
      QObjectList  *list = dialog->queryList( "QLineEdit" );
      QObjectListIt it( *list );
      while ( it.current() )
	{ ((QWidget *) it.current())->setEnabled(FALSE); ++it; }
      delete list;
      dialog->c_type->setEnabled(FALSE);
      dialog->b_ok->setEnabled(FALSE);
      dialog->l_inside->setEnabled(FALSE);
      dialog->l_outside->setEnabled(FALSE);
    }
  dialog->show();
}

void MenuButton::change_accept()
{
  QString prot = "";
  QString str_list = "";
  int i, nr;
  if( dialog )
    {
      if( pmenu_item->getName() != dialog->i_fname->text() )
	if( pmenu_parent->checkFilenames(dialog->i_fname->text()) )
	  {
	    QMessageBox::information(dialog,
				     klocale->translate("Wrong filename"),
				     klocale->translate("A kdelnk-file with this name does already exist. \nPlease choose another filename."),
				     klocale->translate("OK") );
	    return;
	  }
      if( ((QString) dialog->i_fname->text()).isEmpty() )
	{
	  if( ((QString) dialog->i_name->text()).isEmpty() )
	    dialog->i_fname->setText(pmenu_parent->uniqueFileName("menuentry"));
	  else
	    dialog->i_fname->setText(pmenu_parent->uniqueFileName(dialog->i_name->text()));
	}
      EntryType old_type = pmenu_item->getType();
      EntryType new_type = (EntryType) (dialog->c_type->currentItem()+1);
      if( old_type != new_type )
	{
	  if( old_type == submenu )
	    {
	      QMessageBox msg_box;
	      if( msg_box.query( klocale->translate("Warning !"),
				 klocale->translate("Changing the type of this button will delete all of its submenus"),
				 klocale->translate("OK"),
				 klocale->translate("Cancel") ) )
		{
		  popmenu.removeItemAt(0);
		  disconnect( this, SIGNAL(clicked(int)), this, SLOT(sOpen(int)) );
		  pmenu_item->removeMenu();
		}
	      else
		{
		  dialog->show();
		  return;
		}
	    }
	  if( new_type == submenu )
	    {
	      popmenu.insertItem( klocale->translate("Open/ Close"), 0, 0 );
	      popmenu.connectItem( 0, this, SLOT(open()) );
	      connect( this, SIGNAL(clicked(int)), this, SLOT(sOpen(int)) );
	      PMenu *new_menu = new PMenu();
	      new_menu->add( new PMenuItem(unix_com, klocale->translate("EMPTY"),
					   klocale->translate("no command")) );
	      pmenu_item->setMenu( new_menu );
	    }
	}
      pmenu_item->setType( new_type );
      pmenu_item->setText(dialog->i_name->text());
      pmenu_item->setName(dialog->i_fname->text());
      //if(pmenu_item->getName().isEmpty())
      //pmenu_item->setName(pmenu_item->getText());
      setText(pmenu_item->getText());
      pmenu_item->setPixmapName(dialog->i_pixmap->text());
      pmenu_item->setBigPixmapName(dialog->i_big_pixmap->text());
      pmenu_item->setComment(dialog->i_comment->text());
      if( ((QString) dialog->i_pixmap->text()).isEmpty() )
	pmenu_item->setPixmap( global_pix_loader->
			       loadApplicationMiniIcon( dialog->i_big_pixmap->text(), 16, 16 ) );
      else
	pmenu_item->setPixmap( global_pix_loader->
			       loadApplicationMiniIcon( dialog->i_pixmap->text(), 16, 16 ) );
      setPixmap( pmenu_item->getPixmap() );
      switch( (int) new_type ) {
      case (int) unix_com:
	pmenu_item->setCommand(dialog->i_command->text());
	pmenu_item->setExecDir(dialog->i_dir->text());
	pmenu_item->setTermOpt(dialog->i_term_opt->text());
	pmenu_item->setPattern(dialog->i_pattern->text());
	if( dialog->cb_file->isChecked() )
	  prot += "file;";
	if( dialog->cb_ftp->isChecked() )
	  prot += "ftp;";
	if( dialog->cb_http->isChecked() )
	  prot += "http;";
	if( dialog->cb_tar->isChecked() )
	  prot += "tar;";
	if( dialog->cb_info->isChecked() )
	  prot += "info;";
	if( dialog->cb_man->isChecked() )
	  prot += "man;";
	pmenu_item->setProtocols(prot);
	pmenu_item->setUseTerm(dialog->ch_use_term->isChecked());
	nr = dialog->l_inside->count();
	for( i = 0; i < nr; i++ )
	  {
	    str_list += dialog->l_inside->text(i);
	    str_list += ';';
	  }
	pmenu_item->setExtensions(str_list);
	break;
      case (int) swallow_com:
	pmenu_item->setSCommand(dialog->i_swallow_exec->text());
	pmenu_item->setSTitle(dialog->i_swallow_title->text());
	pmenu_item->setCommand(dialog->i_sexec->text());
	pmenu_item->setPattern(dialog->i_pattern->text());
	if( dialog->cb_file->isChecked() )
	  prot += "file;";
	if( dialog->cb_ftp->isChecked() )
	  prot += "ftp;";
	if( dialog->cb_http->isChecked() )
	  prot += "http;";
	if( dialog->cb_tar->isChecked() )
	  prot += "tar;";
	if( dialog->cb_info->isChecked() )
	  prot += "info;";
	if( dialog->cb_man->isChecked() )
	  prot += "man;";
	pmenu_item->setProtocols(prot);
	pmenu_item->setUseTerm(dialog->ch_use_term->isChecked());
	nr = dialog->l_inside->count();
	for( i = 0; i < nr; i++ )
	  {
	    str_list += dialog->l_inside->text(i);
	    str_list += ';';
	  }
	pmenu_item->setExtensions(str_list);
	break;
      case (int) url:
	pmenu_item->setUrl(dialog->i_url->text());
	break;
      case (int) device:
	pmenu_item->setDevice(dialog->i_device->text());
	pmenu_item->setMountP(dialog->i_mount->text());
	pmenu_item->setFSType(dialog->i_fstype->text());
	pmenu_item->setUMPixmapName(dialog->i_mount_pix->text());
	pmenu_item->setDevReadOnly(dialog->cb_read_only->isChecked());
	break;
      };
      ((ConfigureMenu *) parentWidget()->parentWidget())->update();
      delete dialog;
      changes_to_save = TRUE;
      ((KMenuEdit *) KApplication::getKApplication()->mainWidget())->setUnsavedData(TRUE);
      dialog = NULL;
      dialog_open = FALSE;
    }
}

void MenuButton::change_reject()
{
  if( dialog )
    {
      delete dialog;
      dialog = NULL;
      dialog_open = FALSE;
    }
}

void MenuButton::popupMenu(int)
{
  setDown(FALSE);
  repaint(FALSE);
  popmenu.popup(QCursor::pos());
}

void MenuButton::moveEvent( QMoveEvent *)
{
  if( pmenu_item->getMenu() )
    {
      QPoint p = pos() + parentWidget()->parentWidget()->pos();
      QPoint p2(width()-2, 6);
      p += p2;
      pmenu_item->getMenu()->moveConfig(p);
//debug("name = %s", text() );
    }
}

void MenuButton::mousePressEvent( QMouseEvent *e)
{
  if ( isDown() )
    {
debug("is down");
    return;
    }
  bool hit = hitButton( e->pos() );
  if ( hit )
    {                                // mouse press on button
      setDown( TRUE );
      repaint( FALSE );
      if( e->button() == LeftButton && move_mode == MoveNone)
	{
	  emit pressed(id);
	  left_pressed = TRUE;
	  press_x = e->pos().x();
	  press_y = e->pos().y();
	}
      else if( e->button() == MidButton || (e->button() == LeftButton && move_mode != MoveNone))
	{
	  if( (e->state() & ShiftButton) || move_mode == MoveMenu)
	    {
	      move_group = TRUE;
	      setCursor(sizeAllCursor);
	      move_offset = parentWidget()->mapToParent(mapToParent(e->pos()));
	      emit Mpressed(id);
	    }
	  else
	    {
	      if( !(pmenu_item->isReadOnly()) )
		{
		  move_button = TRUE;
		  setCursor(sizeAllCursor);
		  raise();
		  move_offset = e->pos();
		  emit Mpressed(id);
		}
	    }
	  move_mode = MoveNone;
	  QApplication::restoreOverrideCursor();
	}
      else
	emit Rpressed(id);
    }
}

void MenuButton::dndMouseReleaseEvent( QMouseEvent *e)
{
  if ( !isDown() && !move_group && !move_button )
    return;
  bool hit = hitButton( e->pos() );
  setDown( FALSE );
  if( move_button || move_group )
      //  if( e->button() == MidButton )
    {
      setCursor(arrowCursor);
      move_group = FALSE;
      move_button = FALSE;
      emit posChanged(id, pos());
      return;
    }
  if ( hit )
    {                                // mouse release on button
      if ( isToggleButton() )
	setOn( !isOn() );
      repaint( FALSE );
      if ( isToggleButton() )
	emit toggled( isOn() );
      if( e->button() == LeftButton )
	{ left_pressed = FALSE;
	emit released(id);
	emit clicked(id); }
      else if( e->button() == MidButton )
	{ emit Mreleased(id);
	emit Mclicked(id); }
      else
	{ emit Rreleased(id);
	emit Rclicked(id); }
    }
  else
    {
      repaint( FALSE );
      if( e->button() == LeftButton )
	{ left_pressed = FALSE;
	emit released(id); }
      else if( e->button() == MidButton )
	emit Mreleased(id);
      else
	emit Rreleased(id);
    }
}

void MenuButton::dndMouseMoveEvent( QMouseEvent *e )
{
  if( move_group )
    {
      int x, y;
      QPoint p = QCursor::pos();
      p -= move_offset;
      QWidget *par = parentWidget()->parentWidget();
      p = par->parentWidget()->mapFromGlobal(p);
      x = p.x() < 0 ? 0 : p.x();
      y = p.y() < 0 ? 0 : p.y();
      int dw = QApplication::desktop()->width();
      int dh = QApplication::desktop()->height();
      if( (x + width() + 8) > dw )
	x = dw - width() - 8 ;
      if( (y + par->height() + 2) > dh )
	y = dh - par->height() - 2;
      par->move(x, y);
      ((ConfigureMenu *) par)->moveEnable(FALSE);
      return;
    }
  if( move_button )
    {
      int y = mapToParent(e->pos()).y() - move_offset.y();
      if( y < 0 )
	y = 0;
      if( y+height() > parentWidget()->height() )
	y = parentWidget()->height() - height();
      move(0, y);
      return;
    }
  if( left_pressed )
    {
      int x = e->pos().x();
      int y = e->pos().y();

      if ( abs( x - press_x ) > Dnd_X_Precision || abs( y - press_y ) > Dnd_Y_Precision )
	{
	  if( global_drop_buffer )
	    delete global_drop_buffer;
	  global_drop_buffer = new PMenuItem( *pmenu_item );

	  QString data = "kmenuedit:copyEntry";
	  QPoint p = mapToGlobal( e->pos() );
	  QPixmap pix = *(pixmap());
	  int dx = - pix.width() / 2;
	  int dy = - pix.height() / 2;
	  startDrag( new KDNDIcon( pix, p.x() + dx, p.y() + dy ),
	  	   data.data(), data.length(), DndText, dx, dy );
	}
      return;
    }
  bool hit = hitButton( e->pos() );
  if ( hit )
    {                                // mouse move in button
      if ( !isDown() )
	{
	  setDown( TRUE );
	  repaint( FALSE );
	  if( e->button() == LeftButton )
	    emit pressed(id);
	  else if( e->button() == MidButton )
	    emit Mpressed(id);
	  else
	    emit Rpressed(id);
	}
    }
  else
    {                                    // mouse move outside button
      /*
      if ( isDown() )
	{
	  setDown( FALSE );
	  repaint( FALSE );
	  if( e->button() == LeftButton )
	    emit released(id);
	  else if( e->button() == MidButton )
	    emit Mreleased(id);
	  else
	    emit Rreleased(id);
	}
	*/
    }
}

void MenuButton::enterEvent( QEvent * )
{
  if( move_group || move_button )
    return;
  if( isEnabled() )
    raised = 1;
  setFocus();
  repaint();
  global_status_bar->changeItem( (char *) ((const char *) pmenu_item->getComment()), 1 );
}

void MenuButton::leaveEvent( QEvent * )
{
  if( move_group || move_button )
    return;
  raised = 0;
  clearFocus();
  repaint();
  global_status_bar->changeItem( "", 1 );
}

void MenuButton::focusOutEvent( QFocusEvent * )
{
  raised = 0;
  repaint();
}

void MenuButton::drawButton( QPainter *_painter )
{
  paint( _painter );
}

void MenuButton::drawButtonLabel( QPainter *_painter )
{
  paint( _painter );
}

void MenuButton::paint( QPainter *painter )
{
  QColorGroup g = colorGroup();

  if ( raised == 1 )
    {
      if ( style() == WindowsStyle )
        qDrawWinButton( painter, 0, 0, width(), height(),
                        g, FALSE );
      else
	qDrawShadePanel( painter, 0, 0, width(), height(),
                         g, FALSE, 2, 0L );
    }
  else if ( raised == -1 )
    {
      if ( style() == WindowsStyle )
        qDrawWinButton( painter, 0, 0, width(),
                        height(), g, TRUE );
      else
        qDrawShadePanel( painter, 0, 0, width(),
			 height(), g, TRUE, 2, 0L );
    }

  if( pmenu_item->getType() == separator )
    {
      qDrawShadeLine( painter, 0, 2, width(), 2,
                     g, TRUE, 1, 0 );
      return;
    }
  if ( pixmap() )
    {
      //int dx = ( width() - pixmap()->width() ) / 2;
      int dy = ( height() - pixmap()->height() ) / 2;
      //painter->drawPixmap( dx, dy, *pixmap() );
      painter->drawPixmap( 2, dy, *pixmap() );
    }
  if( pmenu_item->getType() == submenu )
    {
      qDrawArrow( painter, RightArrow, style(), submenu_open,
		  width() - 10,  height()/2-2,
		  8, 8, g );
    }
}

//----------------------------------------------------------------------
//---------------  CONFMENUITEM  ---------------------------------------
//----------------------------------------------------------------------

ConfMenuItem::ConfMenuItem(MenuButton *but, PMenuItem *item)
{
  initMetaObject();
  button = but;
  pmenu_item = item;
}

//----------------------------------------------------------------------
//---------------  CONFIGUREMENU  --------------------------------------
//----------------------------------------------------------------------


ConfigureMenu::ConfigureMenu( PMenu *m, QWidget *parent, const char *name )
  : QFrame( parent, name, WStyle_Customize | WStyle_NoBorder )
     //: QFrame( parent, name )
{
  initMetaObject();
  setFrameStyle( QFrame::Panel | QFrame::Raised );
  pmenu = m;
  but_list.setAutoDelete(TRUE);
  prot_group = new QButtonGroup(this, "prot_group");
  prot_group->setFrameStyle( QFrame::NoFrame );
  prot_group->move(3,3);
  but_group = new QButtonGroup(this, "but_group");
  but_group->setFrameStyle( QFrame::NoFrame );
  but_group->move(3,3);
  but_group->hide();
  KDNDDropZone *dropzone = new KDNDDropZone( but_group, DndURL | DndText );
  connect( dropzone, SIGNAL(dropAction(KDNDDropZone *)), this, SLOT(urlDroped(KDNDDropZone *)) );
  but_nr = 0;
  width = 0;
  move_enable = TRUE;
  height = 3;
  group_height = 0;
}

ConfigureMenu::~ConfigureMenu()
{
  ConfMenuItem *item;
  for( item = but_list.first(); item != 0; item = but_list.next() )
    {
      if( item->pmenu_item->getMenu() )
	{
	  item->pmenu_item->getMenu()->hideConfig();
	}
    }
  but_list.clear();
  if( but_group )
    delete but_group;
}

void ConfigureMenu::append(PMenuItem *item)
{
  short but_width, but_height;
  MenuButton *but;
  ConfMenuItem *conf_item;
  but = new MenuButton(item, but_nr, pmenu, item->isReadOnly() ? prot_group : but_group);
  but_width = but->sizeHint().width();
  if( item->getType() == separator )
    but_height = 5;
  else
    but_height = glob_but_height;
  height += but_height;
  if( !(item->isReadOnly()) )
    {
      group_height += but_height;
      but_group->show();
    }
  if( but_width+6 > width )
    {
      width = but_width+6;
      ConfMenuItem *l_but;
      for( l_but = but_list.first(); l_but != 0; l_but = but_list.next() )
	{
	  l_but->button->resize(width-6, l_but->button->height());
	}
    }
  but_group->resize(width-6, group_height);
  prot_group->resize(width-6, height-3);
  resize(width, height+3);
  but->setGeometry( 0, height-but_height-3, width-6, but_height );
  but->show();
  pmenu->posRequest();
  //----
  conf_item = new ConfMenuItem(but, item);
  but_list.append(conf_item);
  connect( but, SIGNAL(posChanged(int,QPoint)), this, SLOT(buttonMoved(int, QPoint)) );
  connect( but, SIGNAL(newButton(int)), this, SLOT(newButton(int)) );
  connect( but, SIGNAL(delButton(int)), this, SLOT(delButton(int)) );
  connect( but, SIGNAL(pasteButton(int)), this, SLOT(pasteButton(int)) );
  but_nr++;
}

void ConfigureMenu::update()
{
  hide();
  QListIterator<ConfMenuItem> it1(but_list);
  QListIterator<ConfMenuItem> it2(but_list);
  short but_width, but_height;
  ConfMenuItem *item;
  height = 3;
  group_height = 0;
  width = 0;
  for( ; it1.current(); ++it1 )
    {
      item = it1.current();
      but_width = item->button->sizeHint().width();
      if( item->pmenu_item->getType() == separator )
	but_height = 5;
      else
	but_height = glob_but_height;
      height += but_height;
      if( !(item->pmenu_item->isReadOnly()) )
	{
	  group_height += but_height;
	  but_group->show();
	}
      if( but_width+6 > width )
	{
	  width = but_width+6;
	  ConfMenuItem *l_but;
	  it2.toFirst();
	  for( ; it2.current(); ++it2 )
	    {
	      l_but = it2.current();
	      l_but->button->resize(width-6, l_but->button->height());
	      l_but->button->move( l_but->button->pos() );
	    }
	}
      but_group->resize(width-6, group_height);
      prot_group->resize(width-6, height-3);
      resize(width, height+3);
      item->button->setGeometry( 0, height-but_height-3, width-6, but_height );
    }
  pmenu->posRequest();
  show();
  QWidget::update();
}

void ConfigureMenu::buttonMoved( int but_id, QPoint p )
{
  ConfMenuItem *item;
  int i;
  int y = p.y();
  int h = 0;
  for( i = 0, item = but_list.first(); item != 0; item = but_list.next(), i++ )
    {
      if( y <= item->button->y() )
	if( i == but_id )
	  continue;
	else
	  break;
      h += item->button->height();
    }
  //debug( "i = %i", i);
  int moved_b_height = but_list.at(but_id)->button->height();
  MenuButton *current = but_list.at(but_id)->button;
  if( but_id > i )
    {
      current->move(0, h);
      current->setId(i);
      for( int j = i; j < but_id; j++ )
	{
	  current = but_list.at(j)->button;
	  current->move( 0, current->pos().y() + moved_b_height ) ;
	  current->setId(j+1);
	}
      item = but_list.take(but_id);
      but_list.insert( i, item);
    }
  else
    {
      current->setId(i-1);
      for( int j = but_id+1; j < i; j++ )
	{
	  current = but_list.at(j)->button;
	  current->move( 0, current->pos().y() - moved_b_height ) ;
	  current->setId(j-1);
	}
      if( i-1 == but_id ) // button was not moved to a new position
	{
	  but_list.at(but_id)->button->move( 0, h);
	}
      else
	{
	  but_list.at(but_id)->button->move( 0, current->pos().y() + current->height() );
	  item = but_list.take(but_id);
	  but_list.insert( i-1, item);
	}
    }
  pmenu->move(but_id, i);
  changes_to_save = TRUE;
  ((KMenuEdit *) KApplication::getKApplication()->mainWidget())->setUnsavedData(TRUE);
  //debug
  //for( item = but_list.first(); item != 0; item = but_list.next() )
  //  {
  //    debug("name = %s / id = %i", item->button->text(), item->button->getId());
  //  }
}

void ConfigureMenu::newButton( int but_id )
{
  PMenuItem *new_it = new PMenuItem;
  new_it->setType(unix_com);
  new_it->setText(klocale->translate("EMPTY"));
  new_it->setCommand(klocale->translate("no command"));
  append(new_it);
  pmenu->add(new_it);
  changes_to_save = TRUE;
  ((KMenuEdit *) KApplication::getKApplication()->mainWidget())->setUnsavedData(TRUE);
  buttonMoved( but_nr-1, but_list.at(but_id)->button->pos() );
  
  but_list.at(but_id)->button->change_item();
}

void ConfigureMenu::delButton( int but_id )
{
  pmenu->remove( but_id );
  but_list.remove( but_id );
  but_nr--;
  ConfMenuItem *item;
  int i = 0;
  for( item = but_list.first(); item != 0; item = but_list.next(), i++ )
    {
      item->button->setId(i);
    }
  update();
}

void ConfigureMenu::pasteButton( int but_id )
{
  if( !global_pmenu_buffer )
    return;
  PMenuItem *new_it = new PMenuItem( *global_pmenu_buffer );
  append( new_it );
  pmenu->add( new_it );
  changes_to_save = TRUE;
  ((KMenuEdit *) KApplication::getKApplication()->mainWidget())->setUnsavedData(TRUE);
  buttonMoved( but_nr-1, but_list.at(but_id)->button->pos() );
}

void ConfigureMenu::moveEvent( QMoveEvent * )
{
  ConfMenuItem *l_but;
  for( l_but = but_list.first(); l_but != 0; l_but = but_list.next() )
    {
      l_but->button->parentMoved();
    }
}

void ConfigureMenu::urlDroped(KDNDDropZone *zone)
{
  QString name;
  PMenuItem *new_item;
  if( zone->getDataType() == DndText )
    {
      name = (QString) zone->getData();
      if( name == "kmenuedit:copyEntry" )
	{
	  if( global_drop_buffer == NULL )
	    return;
	  ConfMenuItem *child;
	  if( global_drop_buffer->getType() != separator )
	    {
	      QString temp = global_drop_buffer->getText();
	      for( child = but_list.first(); child != 0; child = but_list.next() )
		{
		  if( temp == child->pmenu_item->getText() )
		    return;
		}
	    }
	  new_item = new PMenuItem( *global_drop_buffer );
	  append( new_item );
	  pmenu->add( new_item );	
	  changes_to_save = TRUE;
	  ((KMenuEdit *) KApplication::getKApplication()->mainWidget())->setUnsavedData(TRUE);
	}
      return;
    }
  QFileInfo fi;
  if( zone->getDataType() != DndURL )
    return;
  QStrListIterator it(zone->getURLList());
  for( ; it.current(); ++it )
    {
      //debug("URL = %s", it.current());
      name = (QString) it.current();
      if( name.left(5) != "file:" )
	continue;
      fi.setFile( name.mid(5, name.length()) );
      //if( !fi.extension().contains("kdelnk") )
      if( !isKdelnkFile(fi.absFilePath()) )
	{
	  if( fi.isExecutable() )
	    {
	      new_item = new PMenuItem;
	      new_item->setText(fi.fileName());
	      new_item->setCommand(fi.absFilePath());
	      new_item->setType(unix_com);
	      append( new_item );
	      pmenu->add( new_item );
	      changes_to_save = TRUE;
	      ((KMenuEdit *) KApplication::getKApplication()->mainWidget())->setUnsavedData(TRUE);
	      new_item->setName( pmenu->uniqueFileName(fi.fileName()) );
	    }
	  continue;
	}
      new_item = new PMenuItem;
      if( new_item->parse(&fi) < 0 )
	delete new_item;
      else
	{
	  append( new_item );
	  pmenu->add( new_item );
	  changes_to_save = TRUE;
	  ((KMenuEdit *) KApplication::getKApplication()->mainWidget())->setUnsavedData(TRUE);
	  //buttonMoved( but_nr-1, but_list.at(but_id)->button->pos() );
	}
    }
}



