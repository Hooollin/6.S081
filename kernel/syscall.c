#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "syscall.h"
#include "defs.h"

// Fetch the uint64 at addr from the current process.
int
fetchaddr(uint64 addr, uint64 *ip)
{
  struct proc *p = myproc();
  if(addr >= p->sz || addr+sizeof(uint64) > p->sz)
    return -1;
  if(copyin(p->pagetable, (char *)ip, addr, sizeof(*ip)) != 0)
    return -1;
  return 0;
}

// Fetch the nul-terminated string at addr from the current process.
// Returns length of string, not including nul, or -1 for error.
int
fetchstr(uint64 addr, char *buf, int max)
{
  struct proc *p = myproc();
  int err = copyinstr(p->pagetable, buf, addr, max);
  if(err < 0)
    return err;
  return strlen(buf);
}

static uint64
argraw(int n)
{
  struct proc *p = myproc();
  switch (n) {
  case 0:
    return p->trapframe->a0;
  case 1:
    return p->trapframe->a1;
  case 2:
    return p->trapframe->a2;
  case 3:
    return p->trapframe->a3;
  case 4:
    return p->trapframe->a4;
  case 5:
    return p->trapframe->a5;
  }
  panic("argraw");
  return -1;
}

// Fetch the nth 32-bit system call argument.
int
argint(int n, int *ip)
{
  *ip = argraw(n);
  return 0;
}

// Retrieve an argument as a pointer.
// Doesn't check for legality, since
// copyin/copyout will do that.
int
argaddr(int n, uint64 *ip)
{
  *ip = argraw(n);
  return 0;
}

// Fetch the nth word-sized system call argument as a null-terminated string.
// Copies into buf, at most max.
// Returns string length if OK (including nul), -1 if error.
int
argstr(int n, char *buf, int max)
{
  uint64 addr;
  if(argaddr(n, &addr) < 0)
    return -1;
  return fetchstr(addr, buf, max);
}

extern uint64 sys_chdir(void);
extern uint64 sys_close(void);
extern uint64 sys_dup(void);
extern uint64 sys_exec(void);
extern uint64 sys_exit(void);
extern uint64 sys_fork(void);
extern uint64 sys_fstat(void);
extern uint64 sys_getpid(void);
extern uint64 sys_kill(void);
extern uint64 sys_link(void);
extern uint64 sys_mkdir(void);
extern uint64 sys_mknod(void);
extern uint64 sys_open(void);
extern uint64 sys_pipe(void);
extern uint64 sys_read(void);
extern uint64 sys_sbrk(void);
extern uint64 sys_sleep(void);
extern uint64 sys_unlink(void);
extern uint64 sys_wait(void);
extern uint64 sys_write(void);
extern uint64 sys_uptime(void);

uint64 sys_sigalarm(void) {
  int n;
  uint64 p;
  if(argint(0, &n) < 0)
    return -1;
  if(argaddr(1, &p) < 0) {
    return -1;
  }
  //printf("sigalarm: %d\n, %p\n", p, myproc()->trapframe->a1);

  //uint64 pa = walkaddr(myproc()->pagetable, p);
  myproc()->ticks = n;
  myproc()->handler = p;
  //myproc()->handler = (void (*)(void))p;
  
  return 0;
}

void fill_trapframe(void) {
  struct proc *p = myproc();
  p->trapframe->kernel_satp = p->alarm_trapframe->kernel_satp;
  p->trapframe->kernel_sp   = p->alarm_trapframe->kernel_sp;
  p->trapframe->kernel_trap = p->alarm_trapframe->kernel_trap;
  p->trapframe->epc = p->alarm_trapframe->epc;
  p->trapframe->kernel_hartid = p->alarm_trapframe->kernel_hartid;
  p->trapframe->ra = p->alarm_trapframe->ra;
  p->trapframe->sp = p->alarm_trapframe->sp;
  p->trapframe->gp = p->alarm_trapframe->gp;
  p->trapframe->tp = p->alarm_trapframe->tp;
  p->trapframe->t0 = p->alarm_trapframe->t0;
  p->trapframe->t1 = p->alarm_trapframe->t1;
  p->trapframe->t2 = p->alarm_trapframe->t2;
  p->trapframe->s0 = p->alarm_trapframe->s0;
  p->trapframe->s1 = p->alarm_trapframe->s1;
  p->trapframe->a0 = p->alarm_trapframe->a0;
  p->trapframe->a1 = p->alarm_trapframe->a1;
  p->trapframe->a2 = p->alarm_trapframe->a2;
  p->trapframe->a3 = p->alarm_trapframe->a3;
  p->trapframe->a4 = p->alarm_trapframe->a4;
  p->trapframe->a5 = p->alarm_trapframe->a5;
  p->trapframe->a6 = p->alarm_trapframe->a6;
  p->trapframe->a7 = p->alarm_trapframe->a7;
  p->trapframe->s2 = p->alarm_trapframe->s2;
  p->trapframe->s3 = p->alarm_trapframe->s3;
  p->trapframe->s4 = p->alarm_trapframe->s4;
  p->trapframe->s5 = p->alarm_trapframe->s5;
  p->trapframe->s6 = p->alarm_trapframe->s6;
  p->trapframe->s7 = p->alarm_trapframe->s7;
  p->trapframe->s8 = p->alarm_trapframe->s8;
  p->trapframe->s9 = p->alarm_trapframe->s9;
  p->trapframe->s10 = p->alarm_trapframe->s10;
  p->trapframe->s11 = p->alarm_trapframe->s11;
  p->trapframe->t3 = p->alarm_trapframe->t3;
  p->trapframe->t4 = p->alarm_trapframe->t4;
  p->trapframe->t5 = p->alarm_trapframe->t5;
  p->trapframe->t6 = p->alarm_trapframe->t6;
}

uint64 sys_sigreturn(void) {
  fill_trapframe();
  myproc()->in_alarm_handler = 0;
  return 0;
}

static uint64 (*syscalls[])(void) = {
[SYS_fork]        sys_fork,
[SYS_exit]        sys_exit,
[SYS_wait]        sys_wait,
[SYS_pipe]        sys_pipe,
[SYS_read]        sys_read,
[SYS_kill]        sys_kill,
[SYS_exec]        sys_exec,
[SYS_fstat]       sys_fstat,
[SYS_chdir]       sys_chdir,
[SYS_dup]         sys_dup,
[SYS_getpid]      sys_getpid,
[SYS_sbrk]        sys_sbrk,
[SYS_sleep]       sys_sleep,
[SYS_uptime]      sys_uptime,
[SYS_open]        sys_open,
[SYS_write]       sys_write,
[SYS_mknod]       sys_mknod,
[SYS_unlink]      sys_unlink,
[SYS_link]        sys_link,
[SYS_mkdir]       sys_mkdir,
[SYS_close]       sys_close,
[SYS_sigalarm]    sys_sigalarm,
[SYS_sigreturn]   sys_sigreturn,
};

void
syscall(void)
{
  int num;
  struct proc *p = myproc();

  num = p->trapframe->a7;
  if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
    p->trapframe->a0 = syscalls[num]();
  } else {
    printf("%d %s: unknown sys call %d\n",
            p->pid, p->name, num);
    p->trapframe->a0 = -1;
  }
}
