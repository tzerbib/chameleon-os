#include "xlib.h"

char* s = "hi from x4!!\n";

// Example read/write extension at mkdir hookpoint
int EXT_ENTRY_RW(mkdir, char* path){
  xputs(s);
  path[0] = 'a';
  path[1] = 'a';
  path[2] = '\0';
  return 0;
}
