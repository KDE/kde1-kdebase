#include <syscall.h>
#include <linux/kernel.h>
#include <unistd.h>

void KMemoryWidget::update()
{
  struct sysinfo info;

  syscall(SYS_sysinfo, &info);

  // Numerical values
  totalMem->setText(format(info.totalram));
  freeMem->setText(format(info.freeram));
  sharedMem->setText(format(info.sharedram));
  bufferMem->setText(format(info.bufferram));
  swapMem->setText(format(info.totalswap));
  freeSwapMem->setText(format(info.freeswap));
}
