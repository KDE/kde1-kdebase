#include <string.h>
#include <qmsgbox.h>       
#include <signal.h>
#include <stdlib.h>
#include <kapp.h>
#include <qpushbt.h>
#include <qfont.h>
#include <kmenubar.h>
#include <ktopwidget.h>
#include <kpanner.h>
#include <time.h>
#include <kprocess.h>

#include "ksmbstatus.h"
#include "ksmbstatus.moc"

#include "ktablistbox.h"

void NetMon::help()
{
//        KNetMon->invokeHTMLHelp("","");                 

}

void NetMon::processLine(char *bufline, int linelen)
{
    char *tok;
    int pid;

    rownumber++;
    if (rownumber == 2)
        version->setText(bufline); // second line = samba version
    else if ((rownumber >= 5) && (readingpart==connexions))
    {
        char li[255];

        if (linelen<=1)
            readingpart=locked_files; // stop after first empty line.
        else {
            tok= strtok(bufline," ");
            sprintf(li,"%s\n",tok);
            tok= strtok(NULL," ");
            sprintf(li,"%s%s\n",li,tok);      
            tok= strtok(NULL," ");
            sprintf(li,"%s%s\n",li,tok);      
            tok= strtok(NULL," ");
            pid= atoi(tok);
            pids[nrpid++]=pid;
            sprintf(li,"%s%s\n",li,tok);      
            tok= strtok(NULL," ");
            sprintf(li,"%s%s\n",li,tok);      
            
            list->insertItem(li);
        }
      }
    else if (readingpart==locked_files)
    {
      if ((strncmp(bufline,"No",2) == 0) || (linelen<=1))
      { // "No locked files"
          readingpart=finished;
          //debug("finished");
      }
      else
          if ((strncmp(bufline,"Pi", 2) !=0) // "Pid DenyMode ..."
              && (strncmp(bufline,"--", 2) !=0)) // "------------"
          {
            tok=strtok(bufline," ");
            pid=atoi(tok);
            lo[pid]++; 
          }        
    }
}

// called when we get some data from smbstatus
//   can be called for any size of buffer (one line, several lines,
//     half of one ...)
void NetMon::slotReceivedData(KProcess *, char *buffer, int
                              buflen)
{
    char s[250],*start,*end;
    size_t len;
    start = buffer;
    while ((end = index(start,'\n'))) // look for '\n'
    {
        len = end-start;
        strncpy(s,start,len);
        s[len] = '\0';
        //debug(s); debug("**");
        processLine(s,len); // process each line
        start=end+1;
    }
    // here we could save the remaining part of line, if ever buffer
    // doesn't end with a '\n' ... but will this happen ?
}

void NetMon::update()
{
    int n;
    KProcess * process = new KProcess();

    for(n=0;n<=65536;n++) lo[n]=0;
    list->clear();
    /* Re-read the Contents ... */

    rownumber=0;
    readingpart=connexions;
    nrpid=0;
    connect(process, 
            SIGNAL(receivedStdout(KProcess *, char *, int)),
            SLOT(slotReceivedData(KProcess *, char *, int)));
    *process << "smbstatus"; // the command line
    //debug("update");
    if (!process->start(KProcess::Block,KProcess::Stdout)) // run smbstatus
        version->setText(i18n("Error launching smbstatus !"));
    else if (rownumber==0) // empty result
        version->setText(i18n("Error ! smbstatus not found or non working !"));
    else { // ok -> count the number of locked files for each pid
        int m;
        char tmp[255];
        for (m=0;m<nrpid;m++) 
        {
            sprintf(tmp,"%d",lo[pids[m]]);
            list->changeItemPart(tmp,m,5);
        }
    }
    version->adjustSize();
    delete process;
}


void NetMon::Kill()
{
  QString a;
  a = list->text(killrow,3);
  kill(a.toUInt(),15);
  update();
}

void NetMon::Killmenu(int Index, int column)
{
   killrow=Index;
   menu->setTitle("//"+list->text(Index,4)+"/"+list->text(Index,0));
   menu->show();
}

NetMon::NetMon( QWidget * parent, const char * name )
        : KConfigWidget (parent, name)
{

    list=new KTabListBox (this,"Hello",6);
    version=new QLabel(this);
    list->setGeometry(20,20,430,320);
    list->clearTableFlags(Tbl_hScrollBar);
    list->clearTableFlags(Tbl_autoHScrollBar);
    list->setTableFlags(Tbl_autoVScrollBar);
    list->setSeparator('\n');
    list->setColumn(0, i18n("Service"), 80);

    list->setColumn(1, i18n("UID"), 70);
    list->setColumn(2, i18n("GID"), 70);     
    list->setColumn(3, i18n("PID"), 50);
    list->setColumn(4, i18n("Machine"),80);
    list->setColumn(5, i18n("Open Files"),60);
 
    timer = new QTimer(this);
    timer->start(5000);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    menu = new KPopupMenu();
    menu->insertItem("&Kill",this,SLOT(Kill()));
    connect(list,SIGNAL(popupMenu(int,int)),SLOT(Killmenu(int,int)));
    update();

}

void NetMon::resizeEvent( QResizeEvent *re )
{   QSize size = re->size();
    if (list)
        list->setGeometry(SCREEN_XY_OFFSET,SCREEN_XY_OFFSET,
                    size.width() -2*SCREEN_XY_OFFSET,
                    size.height()-10-version->height()-2*SCREEN_XY_OFFSET);
    version->move(SCREEN_XY_OFFSET, SCREEN_XY_OFFSET+list->height()+10);
}                                                                               
