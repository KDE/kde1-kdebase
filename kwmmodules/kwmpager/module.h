
#include <kconfigmodule.h>

#ifndef ModuleName
#define ModuleName KPagerModule
#endif

class KPagerModule : public KConfigModule
{
public:

  KPagerModule();

  virtual void showAboutDialog(QWidget *parent);

  virtual KConfigWidget *getNewWidget(QWidget *parent, const char *name=0, 
				      bool modal=FALSE);
};


