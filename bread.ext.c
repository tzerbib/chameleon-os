#include "xlib_bio.h"

char* s = "hi from bread\n";

int EXT_ENTRY_RO(bread, struct biocontext const* ctx) {
  xputs(s);
  return 0;
}
