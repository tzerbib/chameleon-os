#pragma once

#include "buf.h"

struct biocontext {
  struct buf const* buf; // TODO: flatten this out?
};

struct extcontext;

int build_context_bread(uint const*, uint32_t, struct extcontext*);
int build_context_bflush(uint const*, uint32_t, struct extcontext*);
