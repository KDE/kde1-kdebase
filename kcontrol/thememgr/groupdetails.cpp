/* (C) 1998 Stefan Taferner <taferner@kde.org>
 */

#include "groupdetails.h"
#include "theme.h"
#include "global.h"

//-----------------------------------------------------------------------------
GroupDetails::GroupDetails(const char* aGroupName):
  GroupDetailsInherited(NULL, 0, true)
{
  initMetaObject();

  mGroupName = aGroupName;
  mGroupName.detach();
}


//-----------------------------------------------------------------------------
GroupDetails::~GroupDetails()
{
}


//-----------------------------------------------------------------------------
#include "groupdetails.moc"
