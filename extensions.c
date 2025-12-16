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
  e->hp = HP_none;
  
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

void ext_attach(struct extension* e, enum hookpoint hp) {
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

  e->state = EXT_ATTACHED;
  struct proc *currproc = myproc();
  struct namespace *currns = currproc->ns;
  e->ns = currns;
  attach_ext_to_ns(currns, e);
  e->hp = hp;
  
  release(&exttable.lock);
}

void ext_detach(struct extension* e) {
  // TODO: this function should check if 
  //  multiple ext is attached to this hook

  extern unsigned char nops_start_label;
  extern unsigned char nops_end_label;

  if (e->state != EXT_ATTACHED) {
    return;
  }
  struct proc *currproc = myproc();
  struct namespace *currns = currproc->ns;

  if (e->ns != currns) {
    // Can only detach from current ns?
    cprintf("Cannot detach, wrong ns\n");
    return;
  }

  acquire(&exttable.lock);

  // nops
  asm volatile (
    ".globl nops_start_label\n"
    "nops_start_label:\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    ".globl nops_end_label\n"
    "nops_end_label:\n"
    :
    :
    :);

  unsigned char* hp_start = hptable[e->hp].start;
  // unsigned char* hp_end = hptable[e->hp].end;
   
  // Write nops to replace call trampoline instruction
  memmove(hp_start, &nops_start_label, &nops_end_label - &nops_start_label);

  e->state = EXT_LOADED;
  e->ns = 0x0;
  remove_ext_from_ns(currns, e);
  e->hp = HP_none;
  
  release(&exttable.lock);
}

void trampoline(void) {
  struct proc *currproc = myproc();
  struct namespace *currns = currproc->ns;

  struct ns_object* ns_obj;
  // TODO: figure out how to remove locking
  // acquire(&currns->lock);
  for (ns_obj = currns->namespaced_exts; ns_obj < &currns->namespaced_exts[N_NS_EXT]; ++ns_obj) {
    void* p = ns_obj->pointer;
    struct extension* e = (struct extension*)p;
    if (e->state != EXT_ATTACHED) {
      break;
    }

    e->entry();
  }
  // release(&currns->lock);
}

