#ifndef HASHMAP_H
#define HASHMAP_H

#include "spinlock.h"

#define HMAP_NBUCKETS       64
#define HMAP_KEY_MAX        64
#define HMAP_ENTRIES_PER_SLAB 50

struct hmap_entry {
  char              key[HMAP_KEY_MAX];
  void             *val;
  struct hmap_entry *next;  // chaining within bucket
  int               used;   // 1 = live, 0 = free slot
};

// One kalloc page holds a slab header + 50 entries (3804 of 4096 bytes used)
struct hmap_slab {
  struct hmap_slab  *next;
  struct hmap_entry  entries[HMAP_ENTRIES_PER_SLAB];
};

struct hashmap {
  struct spinlock    lock;
  int                count;
  struct hmap_entry *buckets[HMAP_NBUCKETS];
  struct hmap_slab  *slab;     // current slab (head of slab list)
  int                slab_idx; // next candidate index within current slab
};

// Iterate over every live entry, calling fn(key, val, arg) for each.
// fn must NOT call hm_put/hm_get/hm_del on the same map (deadlock).
typedef void (*hm_iter_fn)(const char *key, void *val, void *arg);

#endif 