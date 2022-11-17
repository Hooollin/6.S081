#include "types.h"
#include "param.h"
#include "riscv.h"
#include "defs.h"
#include "spinlock.h"
#include "proc.h"
#include "sysinfo.h"

uint64 sys_sysinfo(void) {
  struct sysinfo info;
  uint64 addr;
  struct proc *p = myproc();
  if(argaddr(0, &addr) < 0) {
    return -1;
  }
  info.freemem = freemem();
  info.nproc = num_procs();
  if(copyout(p->pagetable, addr, (char *)&info, sizeof(info)) < 0)
    return -1;
  return 0;
}
