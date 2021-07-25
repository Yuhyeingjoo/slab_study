#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int sys_getpriority(void)
{
	int pid;
	int *priority;

	if(argint(0, &pid) == -1){
		return -1;
	}

	if(argptr(1, (char**) &priority, sizeof(int*)) == -1){
		return -1;
	}

	return getpriority(pid, priority);
}


int sys_setpriority(void)
{
	int pid;
	int priority;
	if(argint(0, &pid) == -1){
		return -1;
	}

	if(argint(1, &priority) == -1){
		return -1;
	}

	return setpriority(pid, priority);
}



int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  // EDITED
  int status ;
  if (argint(0, &status) == -1) {
    return -1 ;
  }
  exit(status);
  return status;  // not reached
}

int
sys_wait(void)
{
  int * status ;
  if (argptr(0, (char**)&status, sizeof(int *)) == -1) {
    return -1 ;
  }
  return wait(status);
}

int sys_waitpid(void)
{
  int * status;
  int pid;
  int options;

  if( argint(0, &pid) == -1){
	  return -1;
  }

  if(argptr(1, (char**) &status, sizeof(int*)) == -1){
	 return -1;
  }

  if( argint(2, &options) == -1){
	return -1;
  }

  return waitpid(pid, status, options);  
}
int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
