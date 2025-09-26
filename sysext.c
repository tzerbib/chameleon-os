#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "extensions.h"

// ?? maybe move the following functions to syscall.c
//  depending on if they would be used elsewhere

// Fetch the nth word-sized system call argument as a function pointer
int argfn(int n, void*(**fn)(void)) {
  int i;
 
  if(argint(n, &i) < 0)
    return -1;
  *fn = (void*(*)(void))i;
  return 0;
}

// Fetch the nth word-sized system call argument as a pointer to kernel space
// TODO: probably want to add size check
int argkptr(int n, void** pp) {
  int i;
 
  if(argint(n, &i) < 0)
    return -1;
  *pp = (void**)i;
  return 0;
}


int sys_extload(void) {
  cprintf("hello from sys_extload\n");
  void* (*fn)(void);
  int n;
  struct extension** r;

  if (argint(1, &n) < 0 || argfn(0, &fn) < 0 || argptr(2, (char**)&r, sizeof(r)) < 0) {
    return -1;
  }

  *r = ext_load(fn, n);
  return 0;
};

int sys_extattach(void) {
  cprintf("hello from sys_extattach!\n");
  struct extension* e;

  if (argkptr(0, (void**)&e) < 0) {
    return -1;
  }

  ext_attach(e);

  return 0;
}
