#include <qmsgbox.h>       

#include <stdlib.h>
#include <string.h>
#include <kapp.h>
#include <qpushbt.h>
#include <qfont.h>
#include <kmenubar.h>
#include <ktopwidget.h>
#include <kpanner.h>
#include <time.h>

#include "ksmbstatus.h"
#include "ksmbstatus.moc"

#include "ktablistbox.h"

 KApplication *KNetMon;


void NetMon::help()
{
        KNetMon->invokeHTMLHelp("","");                 

}

void NetMon::update()
{
    FILE *f;
    char tmp[255];
    char li[255];
    char *tok;
    int pid,n,m;
    int pids[1000];
    int lo[65536];
    for(n=0;n<=65536;n++) lo[n]=0;
    list->clear();
    /* Re-read the Contets ... */

    f = popen("smbstatus","r");
    if (f) {
      fgets(tmp,255,f);
      fgets(tmp,255,f);
      fgets(tmp,255,f);
      fgets(tmp,255,f);
      n=0;
      while (strlen(fgets(tmp,255,f)) > 1)
      {
          tok= strtok(tmp," ");
          sprintf(li,"%s\n",tok);
          tok= strtok(NULL," ");
          sprintf(li,"%s%s\n",li,tok);      
          tok= strtok(NULL," ");
          sprintf(li,"%s%s\n",li,tok);      
          tok= strtok(NULL," ");
          pid=atoi(tok);
	  pids[n++]=pid;
          sprintf(li,"%s%s\n",li,tok);      
          tok= strtok(NULL," ");
          sprintf(li,"%s%s\n",li,tok);      

          list->insertItem(li);
      }
      fgets(tmp,255,f);
      fgets(tmp,255,f);
      fgets(tmp,255,f);
      while (strlen(fgets(tmp,255,f)) > 1)
      {
          tok=strtok(tmp," ");
	  pid=atoi(tok);
          lo[pid]++; 
      }        
      pclose(f);
      for (m=0;m<n;m++) 
      {
	  sprintf(tmp,"%d",lo[pids[m]]);
          list->changeItemPart(tmp,m,5);
      }            
    }
}

NetMon::NetMon( QWidget * parent, const char * name)
        : KConfigWidget (parent, name)
{
    list=new KOldTabListBox (this,"Hello",2,2);
    list->setGeometry(20,20,430,280);
    list->clearTableFlags(Tbl_hScrollBar);
    list->clearTableFlags(Tbl_autoHScrollBar);
    list->setTableFlags(Tbl_autoVScrollBar);
    list->setSeparator('\n');
    list->setNumCols(6);
 //   list->setColumn(0, "Type",50);
    list->setColumn(0, "Service", 80);
    list->setColumn(1, "UID", 70);
    list->setColumn(2, "GID", 70);     
    list->setColumn(3, "PID", 50);
    list->setColumn(4, "Machine",80);
    list->setColumn(5, "Open Files",60);
    update();
}
