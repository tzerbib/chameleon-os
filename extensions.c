#include "extensions.h"
#include "types.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"

struct {
  struct spinlock lock;
  struct extension extensions[NEXT];
} exttable;

static struct extension*
ext_load_attach(void* fn)
{
  acquire(&exttable.lock);

  // TODO: Look for an unused extension

  release(&exttable.lock);

  // TODO: Initialize extension

  return (void*)0;
}

static void
trampoline(void)
{
  struct extension* e;

  acquire(&exttable.lock);

  for(e = exttable.extensions; e < &exttable.extensions[NEXT]; ++e) {
    if(e->state != ATTACHED)
      continue;

    // Release extension table lock during extension execution
    release(&exttable.lock);

    e->text();

    acquire(&exttable.lock);
  }

  release(&exttable.lock);

  return;
}

