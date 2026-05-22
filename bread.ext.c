#include "xlib_bio.h"

int EXT_ENTRY_RO(bread, struct biocontext const* ctx) {
  xputs("bread: hello from nsid ");
  xputd(mynsid());
  xputs("!\n");
return 0;
}
