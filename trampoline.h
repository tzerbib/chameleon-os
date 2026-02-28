#pragma once

#include "api/hookpoint.h"

struct trampoline_call {
  unsigned char* start;
  unsigned char* end;
};

void trampoline_call_for(enum hookpoint, struct trampoline_call*);
