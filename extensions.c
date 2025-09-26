#include "extensions.h"
#include "types.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "mmu.h"

struct {
  struct spinlock lock;
  struct extension extensions[NEXT];
} exttable;


struct extension* 
ext_load(void* (*fn)(void), int n) 
{
  cprintf("hello from ext_load!\n");
  
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
  e->text = (void*(*)(void)) page;
  
  safestrcpy(e->name, "read_ext", sizeof(e->name)); // Random hardcoded name for now
  
  release(&exttable.lock);
  return e;
}

void
ext_attach(struct extension* ext)
{
  cprintf("hello from ext_attach!\n");

  extern unsigned char sys_read_nop_start;
  extern unsigned char sys_read_nop_end;
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
  memmove(&sys_read_nop_start, &trampoline_call_start, &trampoline_call_end - &trampoline_call_start);

  // Recalculate the relative address of the trampoline function
  int difference = &trampoline_call_end - &sys_read_nop_end;
  int offset;
  memmove(&offset, &sys_read_nop_start + 1, sizeof(offset));
  offset += difference;
  memmove(&sys_read_nop_start + 1, &offset, sizeof(offset));

  ext->state = EXT_ATTACHED;
  
  release(&exttable.lock);
}


static void
trampoline(void)
{
  cprintf("hello from trampoline!!\n");

  struct extension* e;

  acquire(&exttable.lock);

  for(e = exttable.extensions; e < &exttable.extensions[NEXT]; ++e) {
    if(e->state != EXT_ATTACHED)
      continue;

    // Release extension table lock during extension execution
    release(&exttable.lock);

    cprintf("extension returns: %d\n", e->text());

    acquire(&exttable.lock);
  }

  release(&exttable.lock);

  return;
}

