/**********************************************************************

	--- Dlgedit generated file ---

	File: IconPathDialog.h
	Last generated: Tue Jul 15 17:24:21 1997

 *********************************************************************/

#ifndef IconPathDialog_included
#define IconPathDialog_included

#include <qstring.h>

#include "IconPathDialogData.h"

class IconPathDialog : public IconPathDialogData
{
  Q_OBJECT
public:
  IconPathDialog( QString text, QWidget* parent = NULL, const char* name = NULL );
  virtual ~IconPathDialog();

  QString text();
};
#endif // IconPathDialog_included
