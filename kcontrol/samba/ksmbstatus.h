#ifndef main_included
#define main_included
 
#include "ktablistbox.h"
#include "kcontrol.h"

class NetMon : public KConfigWidget
{
Q_OBJECT
public:
   NetMon::NetMon(QWidget *parent = 0, const char * name=NULL);

   void applySettings() {};
   void loadSettings() {};
   
private:
   KOldTabListBox *list;
private slots:
   void update();
   void help();
protected:
                     
};

#endif // main_included                        
