#pragma once

#include "types.h"

// TODO: rename both to something more clear
// process_context_nsid, data_context_nsids
struct attribution {
  int process; // triggering_nsid
  uint16_t nsids; // data_interest_nsids ? 
};

struct policy {
    struct policy_vtable const* vtable;
};

struct extcontext;

void policy_ctor(struct policy* this, struct policy_vtable const* vtable);
void policy_dtor(struct policy* this);

struct attribution policy_get(struct policy* this, struct extcontext* ctx);

typedef typeof(policy_get) policy_get_t;
typedef typeof(policy_dtor) policy_dtor_t;

// List of virtual functions for policy type
struct policy_vtable {
    policy_get_t* get;
    policy_dtor_t* dtor;
};
 
#define ATTR_HAS_NSID(attr, nsid) ((attr).nsids & (1U << (nsid)))
#define ATTR_SET_NSID(attr, nsid) ((attr).nsids |= (1U << (nsid)))
#define ATTR_RESET_NSIDS(attr) ((attr).nsids = 0)
