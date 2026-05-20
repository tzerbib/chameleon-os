#include "biopolicy.h"

#include "defs.h"
#include "extension/extcontext.h"
#include "policy/policy.h"
#include "proc.h"
#include "namespace.h"
#include "file.h"

static struct attribution biopolicy_get(struct biopolicy const* this, struct extcontext const* ctx) {
  if (ctx == nullptr) {
    panic("ctx is nullptr");
}
  if (ctx->domain != BIO) {
    panic("context is not of type bio");
  }
  struct attribution attr = {
    .process = -1,
    .nsids = 0
  };
  struct proc* currproc = myproc();
  if (currproc != nullptr) {
    attr.process = get_nsid(currproc->ns);
  }
  
  
  const struct buf *b = ctx->bio.buf;
  
  if (b->inode != NULL) {
    // convert ns_opencounts array to bitmap
    for (int i = 0; i < N_NS; i++) {
      if (b->inode->ns_opencounts[i] > 0) {
        attr.nsids |= (1U << i);
      }
    }
  }
  
  return attr;
}

static void biopolicy_dtor(struct biopolicy *this) {}

static struct policy_vtable const biopolicy_vtable = {
    .dtor = (policy_dtor_t*) biopolicy_dtor,
    .get = (policy_get_t*) biopolicy_get,
};

void biopolicy_ctor(struct biopolicy* this) {
    policy_ctor(&this->base, &biopolicy_vtable);
}
