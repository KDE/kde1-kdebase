/* (C) 1998 Stefan Taferner <taferner@kde.org>
 */
#ifndef GROUPDETAILS_H
#define GROUPDETAILS_H

#include <qdialog.h>
#include <ktablistbox.h>
#include <qpushbt.h>

class Theme;

#define GroupDetailsInherited QDialog
class GroupDetails: public QDialog
{
  Q_OBJECT

public:
  GroupDetails(const char* group);
  virtual ~GroupDetails();

protected:
  QString mGroupName;
  QPushButton *btnOk, *btnCancel, *btnAdd, *btnRemove, *btnEdit;
  KTabListBox *tlBox;

protected slots:
  virtual void slotAdd();
  virtual void slotRemove();
  virtual void slotEdit();
  virtual void slotOk();
  virtual void slotCancel();
};


#endif /*GROUPDETAILS_H*/


