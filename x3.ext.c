#include "xlib.h"

char* s = "hi from x3\ncur eip: ";

// Read-only extension at swtch hookpoint
// Prints current context's eip and next's eip
int EXT_ENTRY_RO(swtch, struct context const* const* current, struct context const* next) {
  xputs(s);
  xputx((int)((*current)->eip));
  xputs(", next eip: ");
  xputx((int)(next->eip));
  xputs("\n");
  return 0;
}
