#ifndef main_included
#define main_included
 
#include <ktablistbox.h>
#include <kpopmenu.h>
#include <qtimer.h>
#include <qlabel.h>
#include <kcontrol.h>

class NetMon : public KConfigWidget
{
Q_OBJECT
public:
   NetMon::NetMon(QWidget *parent, const char * name=NULL);

   void applySettings() {};
   void loadSettings() {};    

private:
   KTabListBox *list;
   QLabel *version;
   QTimer *timer;
   KPopupMenu *menu;
   int killrow;
   
private slots:
   void update();
   void help();
   void Kill();
   void Killmenu(int Index, int column);
   
protected:
                     
};

#endif // main_included                        
