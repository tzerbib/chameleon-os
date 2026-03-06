#include "xlib.h"

char* s = "hi from x4!!\n";

// Example read/write extension at mkdir hookpoint
int EXT_ENTRY_RW(mkdir, char* path){
  xputs(s);
  path[0] = 'h';
  path[1] = 'i';
  path[2] = '\0';
  return 0;
}
