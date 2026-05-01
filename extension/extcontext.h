#pragma once

#include "extension/biocontext.h"
#include "policies.h"

struct extcontext {
  enum domain domain;
  union { struct biocontext bio; };
};

int build_context(uint const* ebp, uint32_t ret_addr, struct extcontext* ctx);

typedef typeof(build_context) build_context_t;
