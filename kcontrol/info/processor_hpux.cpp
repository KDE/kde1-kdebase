#include <unistd.h>
#include <sys/param.h>
#include <sys/pstat.h>

KProcessorWidget::KProcessorWidget(QWidget *parent, const char *name)
  : KConfigWidget(parent, name)
{
  long page_size;
  struct pst_dynamic psd;
  struct pst_static  pst;
  QString str;
  char buf[256];
  
  lBox = new QListBox(this);
  lBox->setGeometry(20,20,400,280);
  lBox->setFont(QFont("Courier"));

  if( pstat_getstatic(&pst, sizeof(pst), (size_t)1, 0) == -1 )
    perror("pstat_getstatic");
  page_size =  pst.page_size;

  if( pstat_getdynamic(&psd, sizeof(pst), (size_t)1, 0) == -1 )
    perror("pstat_getstatic");
  
  switch( sysconf(_SC_CPU_VERSION) )
    {
    case CPU_PA_RISC1_0:
      str = "CPU_PA_RISC1_0";
      break;
    case CPU_PA_RISC1_1:
      str = "CPU_PA_RISC1_1";
      break;
    case CPU_PA_RISC2_0:
      str = "CPU_PA_RISC2_0";
      break;
    default:
      str = "unknown CPU type";
      break;
    }
  lBox->insertItem(str);
  
  str = "HOSTNAME= ";
  if( gethostname(buf,256) != 0 )
    str += "unavailable";
  else
    str += buf;
  lBox->insertItem(str);

  lBox->insertItem(QString("Number of active processor(s)= ") + psd.psd_proc_cnt);

  
  
}

