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
  uint32_t nsids; // bitmask on tenants (max 32 for now)
};
#define B_VALID 0x2  // buffer has been read from disk
#define B_DIRTY 0x4  // buffer needs to be written to disk

#define NSID_BIT(nsid)        (1U << (nsid))
#define BUF_HAS_NSID(b, nsid) ((b)->nsids & NSID_BIT(nsid))
#define BUF_ADD_NSID(b, nsid) ((b)->nsids |= NSID_BIT(nsid))
#define BUF_REMOVE_NSID(b, nsid) ((b)->nsids &= ~NSID_BIT(nsid))