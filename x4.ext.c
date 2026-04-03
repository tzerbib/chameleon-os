#include "xlib.h"

char* INNER_MAP_NAME = "syscall_counts";

// Example read/write extension at mkdir hookpoint
// trying to do something like count how many times mkdir is called
int EXT_ENTRY_RO(mkdir, char const* path){
  struct hashmap *maps = my_maps(); 
  
  struct hashmap *syscall_counts = (struct hashmap*)hm_get(maps, INNER_MAP_NAME);
  if (syscall_counts == 0) {
    syscall_counts = hm_alloc();
    if (syscall_counts == 0){
      return 0;
    }
    hm_put(maps, INNER_MAP_NAME, syscall_counts);
  }

  int count = (int)hm_get(syscall_counts, "mkdir"); 

  hm_put(syscall_counts, "mkdir", (void*)(count+1));

  xputs("mkdir count: ");
  xputd((int)hm_get(syscall_counts, "mkdir"));
  xputs("\n");

  return 0;
}
