#include "policy.h"
#include "extension/extcontext.h"

void policy_ctor(struct policy* this, const struct policy_vtable* vtable) {
    this->vtable = vtable;
}

void policy_dtor(struct policy* this) {
    this->vtable->dtor(this);
}

struct attribution policy_get(struct policy* this, struct extcontext* ctx) {
    return this->vtable->get(this, ctx);
}
