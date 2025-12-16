#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  kexit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return kfork();
}
//Lab04 - wait2
uint64
sys_wait2(void)
{
  uint64 status;
  uint64 count;
  
  argaddr(0, &status);  // First argument: exit status pointer
  argaddr(1, &count);   // Second argument: syscall count pointer
  
  return kwait2(status, count);
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  //return kwait(p);
  return kwait2(p, 0);  // Call kwait2 with 0 for the count parameter
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int t;
  int n;

  argint(0, &n);
  argint(1, &t);
  addr = myproc()->sz;

  if(t == SBRK_EAGER || n < 0) {
    if(growproc(n) < 0) {
      return -1;
    }
  } else {
    // Lazily allocate memory for this process: increase its memory
    // size but don't allocate memory. If the processes uses the
    // memory, vmfault() will allocate it.
    if(addr + n < addr)
      return -1;
    myproc()->sz += n;
  }
  return addr;
}

uint64
sys_pause(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kkill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_reboot(void)
{
  volatile uint32 *test_dev = (uint32 *)VIRT_TEST;
  *test_dev = 0x7777;  // Reboot 
  return 0;
}

uint64
sys_quit(void)
{
  volatile uint32 *test_dev = (uint32 *)VIRT_TEST;
  *test_dev = 0x5555;  // Shutdown 
  return 0;
}

uint64
sys_rtcrealtime(void)
{
  volatile uint32 *rtc_base = (uint32 *)GOLDFISH_RTC;
  uint32 first = rtc_base[0];      // Lower 32 bits
  uint32 second = rtc_base[1];     // Upper 32 bits
  
  // Combine into 64-bit timestamp (nanoseconds)
  uint64 timestamp = ((uint64)second << 32) | first;
  
  // Convert to seconds
  return timestamp / 1000000000;
}

uint64 
sys_stracing(void)
{
 struct proc *p = myproc();
 p->stracing = 1;
 return 0;
 
}

uint64
sys_time(void)
{
  return r_time();
}

uint64
sys_nice(void)
{
  int n;
  
  argint(0, &n);
  
  // Clamp to valid range
  if(n < 0) n = 0;
  if(n > 3) n = 3;
  
  struct proc *p = myproc();
  
  acquire(&p->lock);
  p->nice_lv = n;  // Also check: is it nice_lv or nice_level?
  p->priority = 3 - n;
  release(&p->lock);
  
  return 0;
}

uint64
sys_freemem(void)
{
	return freemem();
}
