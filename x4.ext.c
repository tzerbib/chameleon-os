#include "xlib.h"

char* s = "hi from x4\n";

// Example read/write extension at exec hookpoint
// Changes all execs to run `ls`
int EXT_ENTRY_RW(mkdir, char* path, char** argv){
  xputs(s);
  path[0] = 'l';
  path[1] = 's';
  path[2] = '\0';
  argv[0][0] = 'l';
  argv[0][1] = 's';
  argv[0][2] = '\0';
  return 0;
}
