#include "syscallpolicy.h"

#include "ethernet_vlan.h"
#include "extension/extcontext.h"
#include "policy.h"
#include "mmu.h"
#include "param.h"
#include "proc.h"


static struct attribution syscallpolicy_get(struct syscallpolicy*, struct extcontext*) {
  if (myproc() == nullptr) {
    panic("sycallpolicy_get invoked when myproc is nullptr\n");
  }
  return (struct attribution) {
    .process = get_nsid(myproc()->ns),
    .nsids = 0
  };
}

static void syscallpolicy_dtor(struct syscallpolicy*) {}

static struct policy_vtable const syscallpolicy_vtable = {
    .dtor = (policy_dtor_t*) syscallpolicy_dtor,
    .get = (policy_get_t*) syscallpolicy_get,
};

void syscallpolicy_ctor(struct syscallpolicy* this) {
    policy_ctor(&this->base, &syscallpolicy_vtable);
}
