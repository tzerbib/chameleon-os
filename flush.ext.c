#include "types.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "xlib.h"
#include "extension/biocontext.h"

// Fires when a block is evicted from the buffer cache
int EXT_ENTRY_RO(bflush, struct biocontext const* ctx) {
    xputs("hello from bflush! blockno: ");
    xputd(ctx->buf->blockno);
    xputs(" inode=");
    xputd((int)ctx->buf->inode);
    xputs("\n");
    return 0;
}