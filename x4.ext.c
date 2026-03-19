#include "xlib.h"

char* s = "hi from x4!\n";
char* INNER_MAP_NAME = "syscall_counts";

// Example read/write extension at mkdir hookpoint
// trying to do something like count how many times mkdir is called
int EXT_ENTRY_RW(mkdir, char* path){

  xputs(s);

  struct hashmap *maps = my_maps(); 
  
  struct hashmap *counts = (struct hashmap*)hm_get(maps, INNER_MAP_NAME);
  if (counts == 0) {
    counts = hm_alloc();
    if (counts == 0){
      return 0;
    }
    hm_put(maps, INNER_MAP_NAME, counts);
  }

  int* count = (int*)hm_get(counts, "mkdir"); 
  if (count == 0) { 
    // make mkdir KV pair 
    count = (int*) kalloc(); 
    if (count == 0){
      return 0;
    }
    *count = 0; 
    hm_put(counts, "mkdir", count);
  }

  (*count)++;

  xputs("mkdir count: ");
  xputd(*count);
  xputs("\n");

  return 0;
}
