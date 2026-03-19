#include "types.h"
#include "defs.h"
#include "spinlock.h"
#include "mmu.h"
#include "hashmap.h"

// djb2 hash
static uint
hm_hash(const char *key)
{
  uint hash = 5381;
  int c;
  while ((c = (uchar)*key++) != 0)
    hash = hash * 33 ^ c;
  return hash % HMAP_NBUCKETS;
}

// Allocate a new slab page
// returns 0 on oom
static struct hmap_slab*
hm_alloc_slab(void)
{
  struct hmap_slab *s = (struct hmap_slab*)kalloc();
  if (s == 0)
    return 0; 
  memset(s, 0, PGSIZE);
  return s;
}

// Find a free entry slot in hm->slab, allocating a new slab if needed.
// Returns 0 on OOM.
static struct hmap_entry*
hm_alloc_entry(struct hashmap *hm)
{
  struct hmap_slab *s;
  int i;

  // Scan from slab_idx forward in the current slab for a free slot
  if (hm->slab != 0) {
    s = hm->slab;
    for (i = hm->slab_idx; i < HMAP_ENTRIES_PER_SLAB; i++) {
      if (!s->entries[i].used) {
        hm->slab_idx = i + 1;
        return &s->entries[i];
      }
    }
    // Also scan earlier slots (freed entries) from the beginning
    for (i = 0; i < hm->slab_idx; i++) {
      if (!s->entries[i].used)
        return &s->entries[i];
    }
  }

  // Current slab is full (or there is none); allocate a new one
  s = hm_alloc_slab();
  if (s == 0)
    return 0;
  s->next = hm->slab;
  hm->slab = s;
  hm->slab_idx = 1;
  return &s->entries[0];
}

struct hashmap*
hm_alloc(void)
{
  struct hashmap *hm = (struct hashmap*)kalloc();
  if (hm == 0)
    return 0;
  memset(hm, 0, PGSIZE);
  initlock(&hm->lock, "hashmap");
  return hm;
}

void
hm_free(struct hashmap *hm)
{
  struct hmap_slab *s, *next;

  if (hm == 0)
    return;

  s = hm->slab;
  while (s != 0) {
    next = s->next;
    kfree((char*)s);
    s = next;
  }
  kfree((char*)hm);
}

// Insert or update key -> val. Returns 0 on success, -1 on OOM.
int
hm_put(struct hashmap *hm, const char *key, void *val)
{
  uint bucket;
  struct hmap_entry *e;

  acquire(&hm->lock);

  bucket = hm_hash(key);

  // Check for existing key to update in place
  for (e = hm->buckets[bucket]; e != 0; e = e->next) {
    if (strncmp(e->key, key, HMAP_KEY_MAX) == 0) {
      e->val = val;
      release(&hm->lock);
      return 0;
    }
  }

  // New entry
  e = hm_alloc_entry(hm);
  if (e == 0) {
    release(&hm->lock);
    return -1;
  }

  strncpy(e->key, key, HMAP_KEY_MAX - 1);
  e->key[HMAP_KEY_MAX - 1] = '\0';
  e->val = val;
  e->used = 1;
  e->next = hm->buckets[bucket];
  hm->buckets[bucket] = e;
  hm->count++;

  release(&hm->lock);
  return 0;
}

// Look up key. Returns val, or 0 if not found.
void*
hm_get(struct hashmap *hm, const char *key)
{
  uint bucket;
  struct hmap_entry *e;
  void *val;

  acquire(&hm->lock);

  bucket = hm_hash(key);
  val = 0;
  for (e = hm->buckets[bucket]; e != 0; e = e->next) {
    if (strncmp(e->key, key, HMAP_KEY_MAX) == 0) {
      val = e->val;
      break;
    }
  }

  release(&hm->lock);
  return val;
}

// Iterate over every live entry. fn is called with the lock held —
// do not call hm_put/hm_get/hm_del on the same map from within fn.
void
hm_iter(struct hashmap *hm, hm_iter_fn fn, void *arg)
{
  int i;
  struct hmap_entry *e;

  acquire(&hm->lock);
  for (i = 0; i < HMAP_NBUCKETS; i++) {
    for (e = hm->buckets[i]; e != 0; e = e->next)
      fn(e->key, e->val, arg);
  }
  release(&hm->lock);
}

// Remove key. Returns 0 on success, -1 if key not found.
int
hm_del(struct hashmap *hm, const char *key)
{
  uint bucket;
  struct hmap_entry *e, *prev;

  acquire(&hm->lock);

  bucket = hm_hash(key);
  prev = 0;
  for (e = hm->buckets[bucket]; e != 0; prev = e, e = e->next) {
    if (strncmp(e->key, key, HMAP_KEY_MAX) != 0)
      continue;

    // Unlink from bucket chain
    if (prev == 0)
      hm->buckets[bucket] = e->next;
    else
      prev->next = e->next;

    // Mark slot free for reuse; clear key so stale searches can't match
    memset(e->key, 0, HMAP_KEY_MAX);
    e->val = 0;
    e->next = 0;
    e->used = 0;
    hm->count--;

    release(&hm->lock);
    return 0;
  }

  release(&hm->lock);
  return -1;
}
