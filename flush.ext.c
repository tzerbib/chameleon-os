#include "types.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "xlib.h"
#include "extension/biocontext.h"

// Fires when a block is evicted from the buffer cache
int EXT_ENTRY_RO(bflush, struct biocontext const* ctx) {
    xputs("hello from bflush! nsid: ");
    xputd(get_triggering_nsid());
    xputs("\n");
    return 0;
}