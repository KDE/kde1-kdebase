/* (C) 1998 Stefan Taferner <taferner@kde.org>
 */
#ifndef GROUPDETAILS_H
#define GROUPDETAILS_H

#include <qdialog.h>

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
};


#endif /*GROUPDETAILS_H*/


