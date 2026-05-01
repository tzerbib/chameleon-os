#pragma once

#include "policy.h"

struct biopolicy {
  struct policy base;
};

void biopolicy_ctor(struct biopolicy* this);
