// -*- C++ -*-
/**********************************************************************

	--- Qt Architect generated file ---

	File: MenuNameDialog.h
	Last generated: Fri Nov 28 12:05:18 1997

 *********************************************************************/

#ifndef MenuNameDialog_included
#define MenuNameDialog_included

#include "MenuNameDialogData.h"

class MenuNameDialog : public MenuNameDialogData
{
  Q_OBJECT

public:
  MenuNameDialog( QWidget* parent = NULL, const char* name = NULL );
  virtual ~MenuNameDialog();

  void setPersonal(QString text) { i_personal->setText( text ); }
  void setDefault(QString text) { i_default->setText( text ); }
  void setDefaultEnabled(bool b) { i_default->setEnabled( b ); }
  QString getPersonal() { return i_personal->text(); }
  QString getDefault() { return i_default->text(); }

};
#endif // MenuNameDialog_included
