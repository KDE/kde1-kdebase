/* (C) 1998 Stefan Taferner <taferner@kde.org>
 */
#include <qlayout.h>
#include <kapp.h>
#include "groupdetails.h"
#include "theme.h"

//-----------------------------------------------------------------------------
GroupDetails::GroupDetails(const char* aGroupName):
  GroupDetailsInherited(NULL, 0, true)
{
  initMetaObject();

  QBoxLayout *topLayout = new QVBoxLayout(this, 5);
  QBoxLayout *buttonLayout = new QHBoxLayout();

  mGroupName = aGroupName;
  mGroupName.detach();
  tlBox = new KTabListBox(this, "tlBox",2);
  tlBox->setColumn(0,"key", 150);
  tlBox->setColumn(1,"value", 140);
  //tlBox->setFixedSize(310,300);
  topLayout->addWidget(tlBox,10);

  topLayout->addLayout(buttonLayout);

  btnAdd = new QPushButton(i18n("Add"), this);
  btnAdd->setFixedSize(btnAdd->sizeHint());
  buttonLayout->addWidget(btnAdd);
  connect(btnAdd, SIGNAL(clicked()), this, SLOT(slotAdd()));

  btnRemove = new QPushButton(i18n("Remove"), this);
  btnRemove->setFixedSize(btnRemove->sizeHint());
  buttonLayout->addWidget(btnRemove);
  connect(btnRemove, SIGNAL(clicked()), this, SLOT(slotRemove()));

  btnEdit = new QPushButton(i18n("Edit"), this);
  btnEdit->setFixedSize(btnEdit->sizeHint());
  buttonLayout->addWidget(btnEdit);
  connect(btnEdit, SIGNAL(clicked()), this, SLOT(slotEdit()));

  buttonLayout->addStretch(10);

  btnOk = new QPushButton(i18n("OK"), this);
  btnOk->setFixedSize(btnOk->sizeHint() - QSize(6,2));
  connect(btnOk, SIGNAL(clicked()), this, SLOT(slotOk()));
  buttonLayout->addWidget(btnOk);

  btnCancel = new QPushButton(i18n("Cancel"), this);
  btnCancel->setFixedSize(btnCancel->sizeHint() - QSize(6,2));
  connect(btnCancel, SIGNAL(clicked()), this, SLOT(slotCancel()));
  buttonLayout->addWidget(btnCancel);

  topLayout->activate();
}


//-----------------------------------------------------------------------------
GroupDetails::~GroupDetails()
{
}

void GroupDetails::slotAdd()
{
}

void GroupDetails::slotRemove()
{
}

void GroupDetails::slotEdit()
{
}

void GroupDetails::slotOk()
{
close();
}

void GroupDetails::slotCancel()
{
close();
}


//-----------------------------------------------------------------------------
#include "groupdetails.moc"
