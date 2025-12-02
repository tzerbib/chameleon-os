#include "extensions.h"
#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "namespace.h"

typedef void(*function_t)(void);
function_t xtable[] = {
  [0] = (function_t)kalloc,
  [1] = (function_t)kfree,
  [2] = (function_t)cprintf,
};

struct {
  struct spinlock lock;
  struct extension extensions[NEXT];
} exttable;

extern char end[];
char* n_ext = end;

struct extension* ext_load_elf(char* path) {
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

struct extension* ext_load(void* (*fn)(void), int n) {
  // Check if function size is bigger than a page
  if (n < 0 || n > PGSIZE) {
    return (void*)0;
  }
  
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
  
  // Allocate a page to copy the function from user space to kernel space
  // TODO: this could be definitely optimized
  char* page = kalloc();
  memmove(page, fn, n);
  e->entry = (void*(*)(void)) page;
  
  safestrcpy(e->name, "read_ext", sizeof(e->name)); // Random hardcoded name for now
  
  release(&exttable.lock);
  return e;
}

void ext_attach(struct extension* ext) {
  cprintf("hello from ext_attach!\n");

  // Change labels here to attach to a different syscall
  extern unsigned char sys_exec_nop_start;
  extern unsigned char sys_exec_nop_end;
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

  // Write call trampoline instruction to replace nops
  memmove(&sys_exec_nop_start, &trampoline_call_start, &trampoline_call_end - &trampoline_call_start);

  // Recalculate the relative address of the trampoline function
  int difference = &trampoline_call_end - &sys_exec_nop_end;
  int offset;
  memmove(&offset, &sys_exec_nop_start + 1, sizeof(offset));
  offset += difference;
  memmove(&sys_exec_nop_start + 1, &offset, sizeof(offset));

  ext->state = EXT_ATTACHED;
  struct proc *currproc = myproc();
  struct namespace *currns = currproc->ns;
  ext->ns = currns;
  attach_ext_to_ns(currns, ext);
  
  release(&exttable.lock);
}


void trampoline(void) {
  // cprintf("hello from trampoline!!\n");

  struct extension* e;

  struct proc *currproc = myproc();
  struct namespace *currns = currproc->ns;

  acquire(&exttable.lock);

  for(e = exttable.extensions; e < &exttable.extensions[NEXT]; ++e) {
    if(e->state != EXT_ATTACHED) {
      continue;
    }

    if (e->ns != currns) {
      continue;
    }

    // Release extension table lock during extension execution
    release(&exttable.lock);

    // cprintf("extension returns: %d\n", e->text());
    e->entry();

    acquire(&exttable.lock);
  }

  release(&exttable.lock);
}

