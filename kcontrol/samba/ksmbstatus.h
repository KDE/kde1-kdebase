#ifndef main_included
#define main_included
 
#include <ktablistbox.h>
#include <kpopmenu.h>
#include <qtimer.h>
#include <qlabel.h>
#include <kcontrol.h>
#include <kprocess.h>

#define SCREEN_XY_OFFSET 20

class KTabListBox;

class NetMon : public KConfigWidget
{
Q_OBJECT
public:
   NetMon(QWidget *parent, const char * name=NULL);

   void applySettings() {};
   void loadSettings() {};    

private:
   KTabListBox *list;
   QLabel *version;
   QTimer *timer;
   KPopupMenu *menu;
   int killrow;
   int rownumber;
   enum {connexions, locked_files, finished} readingpart;
   int pids[1000];
   int lo[65536]; // nr of locked files for each pid
   int nrpid;
   void processLine(char *bufline, int linelen);
   
private slots:
   void update();
   void help();
   void Kill();
   void Killmenu(int Index, int column);
   void slotReceivedData(KProcess *proc, char *buffer, int buflen);
   
protected:
  virtual void resizeEvent( QResizeEvent * );
                   
};

#endif // main_included                        
