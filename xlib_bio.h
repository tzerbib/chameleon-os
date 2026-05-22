#ifndef XLIB_BIO_H
#define XLIB_BIO_H

#include "extension/biocontext.h"
#include "xlib.h"

// Read-only hookpoints
int EXT_ENTRY_RO(bread, struct biocontext const* ctx);
int EXT_ENTRY_RO(bflush, struct biocontext const* ctx);

#endif // XLIB_BIO_H
