/**********************************************************************

	--- Dlgedit generated file ---

	File: DesktopPathDialog.cpp
	Last generated: Tue Jul 15 18:07:10 1997

 *********************************************************************/

#include "DesktopPathDialog.h"
#include "DesktopPathDialog.moc"
#include "DesktopPathDialogData.moc"

DesktopPathDialog::DesktopPathDialog( QString path, QString ppath, 
				      QWidget* parent, const char* name )
	: DesktopPathDialogData( parent, name )
{
  initMetaObject();
  le_path->setText(path);
  le_path->setFocus();
  le_ppath->setText(ppath);

  connect( b_ok, SIGNAL(pressed()), this, SLOT(accept()) );
}

DesktopPathDialog::~DesktopPathDialog()
{

}

QString DesktopPathDialog::getPath()
{
  return le_path->text();
}

QString DesktopPathDialog::getPPath()
{
  return le_ppath->text();
}
