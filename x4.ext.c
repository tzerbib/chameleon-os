#include "xlib.h"

char* INNER_MAP_NAME = "syscall_counts";

// Example read/write extension at mkdir hookpoint
// trying to do something like count how many times mkdir is called
int EXT_ENTRY_RO(mkdir, char const* path){
  xputs("1\n");
  struct hashmap *maps = my_maps(); 
  xputs("2 ");
  xputx((int)maps); 
  xputs("\n");
  
  struct hashmap *syscall_counts = (struct hashmap*)hm_get(maps, INNER_MAP_NAME);
  xputs("3\n");
  if (syscall_counts == 0) {
    xputs("3.5\n");
    syscall_counts = hm_alloc();
    xputs("4\n");
    if (syscall_counts == 0){
      return 0;
    }
    hm_put(maps, INNER_MAP_NAME, syscall_counts);
    xputs("5\n");
  }

  int count = (int)hm_get(syscall_counts, "mkdir\n"); 
  xputs("6\n");

  hm_put(syscall_counts, "mkdir", (void*)(count+1));

  xputs("mkdir count: ");
  xputd(count);
  xputs("\n");

  return 0;
}
