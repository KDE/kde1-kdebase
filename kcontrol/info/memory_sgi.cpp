#include <stdio.h>
#include <unistd.h>
#include <sys/sysmp.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/param.h>
#include <sys/swap.h>


void KMemoryWidget::update()
{
  int pagesize = getpagesize();

  struct rminfo rmi;
  if( sysmp(MP_SAGET, MPSA_RMINFO, &rmi, sizeof(rmi)) == -1 )
    return;
  totalMem->setText(format(rmi.physmem*pagesize));
  freeMem->setText(format(rmi.freemem*pagesize));
  bufferMem->setText(format(rmi.bufmem*pagesize));

  long val;
  swapctl(SC_GETSWAPTOT, &val);
  swapMem->setText(format(val*UBSIZE));

  swapctl(SC_GETFREESWAP, &val);
  freeSwapMem->setText(format(val*UBSIZE));

  FILE *kmem = fopen("/dev/kmem", "r");
  if( kmem == 0 ) {
    sharedMem->setText(klocale->translate("Not Available"));
    return;
  }

  long shmip = sysmp(MP_KERNADDR, MPKA_SHMINFO);
  fseek( kmem, shmip, 0 );
  struct shminfo shmi;
  fread( &shmi, sizeof(shmi), 1, kmem );

  long shmem = sysmp(MP_KERNADDR, MPKA_SHM);

  val = 0;
  long pos;
  struct shmid_ds shmid;
  for( int i=0 ; i<shmi.shmmni ; i++ ) {
    fseek( kmem, shmem, 0 );
	shmem += sizeof(shmem);
    fread( &pos, sizeof(shmem), 1, kmem );
	if(pos != 0) {
      fseek( kmem, pos, 0 );
      fread( &shmid, sizeof(shmid), 1, kmem );
      val += shmid.shm_segsz;
    }
  }
  sharedMem->setText(format(val));

  fclose(kmem);
}
