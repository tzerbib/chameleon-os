#include "biopolicy.h"

#include "defs.h"
#include "extension/extcontext.h"
#include "policy/policy.h"
#include "proc.h"

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
  attr.nsids = ctx->bio.buf->nsids;
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
