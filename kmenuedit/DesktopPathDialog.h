/**********************************************************************

	--- Dlgedit generated file ---

	File: DesktopPathDialog.h
	Last generated: Tue Jul 15 18:07:10 1997

 *********************************************************************/

#ifndef DesktopPathDialog_included
#define DesktopPathDialog_included

#include "DesktopPathDialogData.h"

class DesktopPathDialog : public DesktopPathDialogData
{
    Q_OBJECT
public:
    DesktopPathDialog( QString path, QString ppath, 
		       QWidget* parent = NULL, const char* name = NULL );
    virtual ~DesktopPathDialog();

    QString getPath();
    QString getPPath();
};
#endif // DesktopPathDialog_included
