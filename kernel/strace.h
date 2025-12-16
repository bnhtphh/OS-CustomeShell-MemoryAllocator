#include "defs.h"
#include "syscall.h"
#pragma GCC diagnostic ignored "-Wunused-function"

static uint64
read_int(int n)
{
  int i;
  argint(n, &i);
  return i;
}

static void *
read_ptr(int n)
{
  uint64 p;
  argaddr(n, &p);
  return (void *) p;
}

static char *
read_str(int n)
{

  static char path[64];
  if(argstr(n, path, 64) < 0) {
    return "";
  }
  return path;
}

void
strace(struct proc *p, int syscall_num, uint64 ret_val)
{
  printf("[strace] syscall: %d\n", syscall_num);
  // This will drop the raw syscall output here, so it's commented out for
  // now... we need to update this function so that it prints system call
  // tracing information (based on the syscall_num passed in).
  
  switch (syscall_num) {
    case SYS_fork:
        printf("[%d|%s] fork() = %ld\n", p->pid, p->name, ret_val);
        break;
    case SYS_exit:
        printf("[%d|%s] exit(int = %ld) = %ld\n", p->pid, p->name, read_int(0), ret_val);
        break;
    case SYS_wait:
        printf("[%d|%s] wait(int = %p) = %ld\n", p->pid, p->name, read_ptr(0), ret_val);
        break;
    case SYS_pipe:
        printf("[%d|%s] pipe(fd = %p) = %ld\n", p->pid, p->name, read_ptr(0), ret_val);
        break;
    case SYS_read:
        printf("[%d|%s] read(fd = %ld, buf = %p, count = %ld) = %ld\n", p->pid, p->name, read_int(0), read_ptr(1), read_int(2), ret_val);
        break;
    case SYS_write:
        printf("[%d|%s] write(fd = %ld, buf = %p, count = %ld) = %ld\n", p->pid, p->name, read_int(0), read_ptr(1), read_int(2), ret_val);
        break;
    case SYS_close:
        printf("[%d|%s] close(fd = %ld) = %ld\n", p->pid, p->name, read_int(0), ret_val);
        break;
    case SYS_kill:
        printf("[%d|%s] kill(int = %ld) = %ld\n", p->pid, p->name, read_int(0), ret_val);
        break;
    case SYS_exec:
        printf("[%d|%s] exec(pathname = %p, argv = %p) = %ld\n", p->pid, p->name, read_str(0), read_ptr(1), ret_val);
        break;
    case SYS_open:
        printf("[%d|%s] open(pathname = %p, mode = %ld) = %ld\n", p->pid, p->name, read_str(0), read_int(1), ret_val);
        break;
    case SYS_mknod:
        printf("[%d|%s] mknod(pathname = %p, short = %ld, short = %ld) = %ld\n", p->pid, p->name, read_ptr(0), read_int(1), read_int(2), ret_val);
        break;
    case SYS_unlink:
        printf("[%d|%s] unlink(pathname = %p) = %ld\n", p->pid, p->name, read_ptr(0), ret_val);
        break;
    case SYS_fstat:
        printf("[%d|%s] fstat(fd = %ld, stat* = %p) = %ld\n", p->pid, p->name, read_int(0), read_ptr(1), ret_val);
        break;
    case SYS_link:
        printf("[%d|%s] link(pathname = %p, char* = %p) = %ld\n", p->pid, p->name, read_ptr(0), read_ptr(1), ret_val);
        break;
    case SYS_mkdir:
        printf("[%d|%s] mkdir(pathname* = %p) = %ld\n", p->pid, p->name, read_ptr(0), ret_val);
        break;
    case SYS_chdir:
        printf("[%d|%s] chdir(pathname = %p) = %ld\n", p->pid, p->name, read_ptr(0), ret_val);
        break;
    case SYS_dup:
        printf("[%d|%s] dup(int = %ld) = %ld\n", p->pid, p->name, read_int(0), ret_val);
        break;
    case SYS_getpid:
        printf("[%d|%s] getpid() = %ld\n", p->pid, p->name, ret_val);
        break;
    case SYS_sbrk:
        printf("[%d|%s] sbrk(int = %ld) = %p\n", p->pid, p->name, read_int(0), (void *) ret_val);
        break;
    case SYS_stracing:
        printf("[%d|%s] strace_on() = %ld\n", p->pid, p->name, ret_val);
        break;
    default:
        printf("[%d|%s] syscall %d = %ld\n", p->pid, p->name, syscall_num, ret_val);
        break;
}
}

