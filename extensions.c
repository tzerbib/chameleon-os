#include "extensions.h"
#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "namespace.h"
#include "api/hookpoint.h"
#include "hookpoints.h"

int mypid(void) {
  return myproc()->pid;
}

typedef void(*function_t)(void);
function_t xtable[] = {
  [0] = (function_t)kalloc,
  [1] = (function_t)kfree,
  [2] = (function_t)cprintf,
  [3] = (function_t)mypid,
};

struct {
  struct spinlock lock;
  struct extension extensions[NEXT];
} exttable;

extern char end[];
char* n_ext = end;

struct extension* ext_load(char* path) {
  acquire(&exttable.lock);

  struct extension* e;

  // Look for an unused extension
  for (e = exttable.extensions; e < &exttable.extensions[NEXT]; e++) {
    if (e->state == EXT_UNUSED) {
      goto found;
    }
  }
  
  release(&exttable.lock);
  return (void*)0;
  
  // Initialize extension
  found:
  e->state = EXT_LOADED;
  
  release(&exttable.lock);
 
  uint path_len = strlen(path);
  if (path_len < sizeof(e->name)) {
    safestrcpy(e->name, path, sizeof(e->name));
  } else {
    safestrcpy(e->name, path + (path_len - sizeof(e->name) + 1), sizeof(e->name));
  }
  
  // This call updates n_ext
  kload_elf(path, &n_ext, (void**)&e->entry);

  return e;
}

void ext_attach(struct extension* ext, enum hookpoint hp) {
  // Change labels here to attach to a different syscall
 
  extern unsigned char trampoline_call_start;
  extern unsigned char trampoline_call_end;

  acquire(&exttable.lock);

  // `call trampoline` instruction as static data
  asm volatile (
    "jmp trampoline_call_end\n"
    ".globl trampoline_call_start\n"
    "trampoline_call_start:\n"
    "call trampoline\n"
    ".globl trampoline_call_end\n"
    "trampoline_call_end:\n"
    :
    :
    :);

  unsigned char* hp_start = hptable[hp].start;
  unsigned char* hp_end = hptable[hp].end;
   
  // Write call trampoline instruction to replace nops
  memmove(hp_start, &trampoline_call_start, &trampoline_call_end - &trampoline_call_start);

  // Recalculate the relative address of the trampoline function
  int difference = &trampoline_call_end - hp_end;
  int offset;
  memmove(&offset, hp_start + 1, sizeof(offset));
  offset += difference;
  memmove(hp_start + 1, &offset, sizeof(offset));

  ext->state = EXT_ATTACHED;
  struct proc *currproc = myproc();
  struct namespace *currns = currproc->ns;
  ext->ns = currns;
  attach_ext_to_ns(currns, ext);
  
  release(&exttable.lock);
}


void trampoline(void) {

  struct proc *currproc = myproc();
  struct namespace *currns = currproc->ns;

  struct ns_object* ns_obj;
  acquire(&currns->lock);
  for (ns_obj = currns->namespaced_exts; ns_obj < &currns->namespaced_exts[N_NS_EXT]; ++ns_obj) {
    void* p = ns_obj->pointer;
    struct extension* e = (struct extension*)p;
    if (e->state != EXT_ATTACHED) {
      continue;
    }

    e->entry();
  }
  release(&currns->lock);
}

