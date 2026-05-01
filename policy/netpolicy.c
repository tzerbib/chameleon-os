#include "netpolicy.h"

#include "ethernet_vlan.h"
#include "extension/extcontext.h"
#include "policy.h"
#include "stack.h"
#include "proc.h"
#include "defs.h"


static struct attribution netpolicy_get(struct netpolicy const* this, struct extcontext*) {
  struct attribution attr = {
    .process = -1,
    .nsids = 0
  };
  if (myproc() == nullptr) {
    ATTR_SET_NSID(attr, stack_top(this->stack));
  } else {
    attr.process = get_nsid(myproc()->ns);
  }
  return attr;
}

static void netpolicy_dtor(struct netpolicy* this) {}

static struct policy_vtable const netpolicy_vtable = {
    .dtor = (policy_dtor_t*) netpolicy_dtor,
    .get = (policy_get_t*) netpolicy_get,
};

void netpolicy_ctor(struct netpolicy* this) {
    policy_ctor(&this->base, &netpolicy_vtable);
    this->stack = &vlan_stack;
}
