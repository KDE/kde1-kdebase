/**********************************************************************

	--- Dlgedit generated file ---

	File: IconPathDialog.cpp
	Last generated: Tue Jul 15 17:24:21 1997

 *********************************************************************/

#include "IconPathDialog.h"
#include "IconPathDialog.moc"
#include "IconPathDialogData.moc"

IconPathDialog::IconPathDialog(	QString text, QWidget* parent, const char* name )
	: IconPathDialogData( parent, name )
{
  initMetaObject();
  le_icon_path->setText(text);
  le_icon_path->setFocus();
  connect( b_ok, SIGNAL(pressed()), this, SLOT(accept()) );
}

IconPathDialog::~IconPathDialog()
{

}

QString IconPathDialog::text()
{
  return le_icon_path->text();
}
