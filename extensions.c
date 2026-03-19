#include "extensions.h"
#include "trampoline.h"
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
  [4] = (function_t)hm_alloc,
  [5] = (function_t)hm_free,
  [6] = (function_t)hm_put,
  [7] = (function_t)hm_get,
  [8] = (function_t)hm_del,
  [9] = (function_t)hm_iter,
};

struct {
  struct spinlock lock;
  struct extension extensions[NEXT];
} exttable;

extern char end[];
char* n_ext = end;

struct extension* ext_load(char* path) {
  acquire(&exttable.lock);

  struct extension* ep;

  // Look for an unused extension
  for (ep = exttable.extensions; ep < &exttable.extensions[NEXT]; ep++) {
    if (ep->state == EXT_UNUSED) {
      goto found;
    }
  }
  
  release(&exttable.lock);
  return (void*)0;
  
  // Initialize extension
  found:
  ep->state = EXT_LOADED;
  ep->hp = HP_none;
  
  release(&exttable.lock);
 
  uint path_len = strlen(path);
  if (path_len < sizeof(ep->name)) {
    safestrcpy(ep->name, path, sizeof(ep->name));
  } else {
    safestrcpy(ep->name, path + (path_len - sizeof(ep->name) + 1), sizeof(ep->name));
  }
  
  // This call updates n_ext
  kload_elf(path, &n_ext, ep);

  cprintf("entry is at %p\n", ep->entry);

  return ep;
}

void ext_attach(struct extension* e) {
  acquire(&exttable.lock);

  enum hookpoint hp = e->hp;
  cprintf("attach, hp %d\n", hp);
  unsigned char* hp_start = hptable[hp].start;
  unsigned char* hp_end = hptable[hp].end;

  struct trampoline_call tc;
  trampoline_call_for(hp, &tc);
  unsigned char* trampoline_call_start = tc.start;
  unsigned char* trampoline_call_end = tc.end;
   
  // Write call trampoline instruction to replace nops
  memmove(hp_start, trampoline_call_start, trampoline_call_end - trampoline_call_start);

  // Recalculate the relative address of the trampoline function
  int difference = trampoline_call_end - hp_end;
  int offset;
  memmove(&offset, hp_start + 2, sizeof(offset));
  offset += difference;
  memmove(hp_start + 2, &offset, sizeof(offset));

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

  // FIXME: labels should be unique to hp?
  asm volatile (
    ".globl nops_start_label\n"
    "nops_start_label:\n"
    "nop\n"
    "nop\n"
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

enum hookpoint check_hookpoint(char const* name) {
  char const* names[] = {
    [HP_getpid] = "getpid",
    [HP_read] = "read",
    [HP_exec] = "exec",
    [HP_swtch] = "swtch",
    [HP_mkdir] = "mkdir", 
  };
  uint name_len = strlen(name);
  for (char const** n = names; n != names + (sizeof(names) / sizeof(names[0])); n++) {
    if (strncmp(*n, name, name_len + 1) == 0) {
      return (enum hookpoint)(n - names);
    }
  }
  panic("No %s hookpoint found\n");
}
