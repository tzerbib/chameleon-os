#include "namespace.h"

struct buf {
  int flags;
  uint dev;
  uint blockno;
  struct sleeplock lock;
  uint refcnt;
  struct buf *prev; // LRU cache list
  struct buf *next;
  struct buf *qnext; // disk queue
  uchar data[BSIZE];
  uint32_t nsids; // bitmask where nsids[i] = 1 if tenant is associated with buf, 0 otherwise
};
#define B_VALID 0x2  // buffer has been read from disk
#define B_DIRTY 0x4  // buffer needs to be written to disk

// check if tenant is associated with this buf 
#define BUF_HAS_NSID(b, nsid) ((b)->nsids & (1U << (nsid)))

// mark that tenant is associated with this buf 
#define BUF_SET_NSID(b, nsid) ((b)->nsids |= (1U << (nsid)))

// clear all tenants from the bitmap 
#define BUF_RESET_NSIDS(b) ((b)->nsids = 0)