
#include <stdio.h>
#include <qobject.h>
#include <qlabel.h>
#include <kmsgbox.h>
#include <klocale.h>
#include <kapp.h>
#include "module.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

KPagerModule::KPagerModule()
  : KConfigModule("pager")
{
  setName(klocale->translate("KWM Pager Properties"));
  setGroup(klocale->translate("Desktop"));
  setHint(klocale->translate("sets the properties for the KWM Pager"));
  setIcon("mini-pager.xpm");
}


void KPagerModule::showAboutDialog(QWidget *parent)
{
  KMsgBox::message(parent, klocale->translate("Pager Module"), 
		   klocale->translate("KWM Pager Modules\n\nby Stephan Kulow."),
		   KMsgBox::INFORMATION, klocale->translate("Close"));
}


KConfigWidget *KPagerModule::getNewWidget(QWidget *parent, const char *name,
					   bool modal)
{
  return new KConfigWidget(parent, name, modal);
}

#ifdef HAVE_DYNAMIC_LOADING

extern "C" KConfigModule *getNewModule()
{
  return new KPagerModule();
}

#endif

