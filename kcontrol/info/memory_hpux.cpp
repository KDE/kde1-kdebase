#include <stdio.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/pstat.h>


void KMemoryWidget::update()
{
  long page_size;
  struct pst_dynamic psd;
  struct pst_static  pst;
  
  if( pstat_getstatic(&pst, sizeof(pst), (size_t)1, 0) == -1 )
    perror("pstat_getstatic");

  if( pstat_getdynamic(&psd, sizeof(pst), (size_t)1, 0) == -1 )
    perror("pstat_getstatic");
  
  page_size = pst.page_size*pst.page_size/1024;
  
     totalMem->setText(format(psd.psd_rm*page_size));
      freeMem->setText(format(psd.psd_free*page_size));
    bufferMem->setText(klocale->translate("Not Available"));
      swapMem->setText(klocale->translate("Not Available"));
  freeSwapMem->setText(klocale->translate("Not Available"));
    sharedMem->setText(klocale->translate("Not Available"));

}
