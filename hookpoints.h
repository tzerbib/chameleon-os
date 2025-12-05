#pragma once
#include "api/hookpoint.h" // IWYU pragma: keep

#define HPLABELS(name)\
  extern unsigned char sys_##name##_nop_start;\
  extern unsigned char sys_##name##_nop_end;

HPLABELS(exec)
HPLABELS(getpid)
HPLABELS(read)

#define HPTENTRY(name)\
  [HP_##name] = {\
    .start = &sys_##name##_nop_start,\
    .end = &sys_##name##_nop_end,\
  }

struct {
  unsigned char* start;
  unsigned char* end;
} hptable[] = {
  HPTENTRY(getpid),
  HPTENTRY(exec),
  HPTENTRY(read),
};
