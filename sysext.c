#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "extensions.h"
#include "api/hookpoint.h"

// ?? maybe move the following functions to syscall.c
//  depending on if they would be used elsewhere

// Fetch the nth word-sized system call argument as a function pointer
int argfn(int n, void*(**fn)(void)) {
  int i;
 
  if(argint(n, &i) < 0) {
    return -1;
  }
  *fn = (void*(*)(void))i;
  return 0;
}

// Fetch the nth word-sized system call argument as a pointer to kernel space
// TODO: probably want to add size check
int argkptr(int n, void** pp) {
  int i;
 
  if(argint(n, &i) < 0) {
    return -1;
  }
  *pp = (void**)i;
  return 0;
}

int sys_extattach(void) {
  struct extension* e;
  int hp;

  if (argkptr(0, (void**)&e) < 0 || argint(1, &hp) < 0) {
    return -1;
  }

  ext_attach(e, (enum hookpoint)hp);
  
  return 0;
}

int sys_extload(void) {
  char* path;
  struct extension** r;

  if (argstr(0, &path) < 0 || argptr(1, (char**)&r, sizeof(r)) < 0) {
    return -1;
  }

  *r = ext_load(path);
  return 0;
}
