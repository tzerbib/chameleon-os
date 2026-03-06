#include "hookpoints.h"

#define HPLABELS(name)\
  extern unsigned char sys_##name##_nop_start;\
  extern unsigned char sys_##name##_nop_end;

HPLABELS(exec)
HPLABELS(getpid)
HPLABELS(read)
HPLABELS(swtch)
HPLABELS(mkdir)

#define HPTENTRY(name)\
  [HP_##name] = {\
    .start = &sys_##name##_nop_start,\
    .end = &sys_##name##_nop_end,\
  }

struct hpt_entry hptable[] = {
  HPTENTRY(getpid),
  HPTENTRY(exec),
  HPTENTRY(read),
  HPTENTRY(swtch),
  HPTENTRY(mkdir),
};
