#include "xlib_bio.h"

int EXT_ENTRY_RO(bread, struct biocontext const* ctx) {
  xputs("bread: hello from nsid ");
  xputd(get_triggering_nsid());
  xputs("!\n");
return 0;
}
