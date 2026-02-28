#include "xlib.h"

char* s = "hi from x2\npath: ";

// Read-only extension at exec hookpoint
// Prints path and first arg
int EXT_ENTRY_RO(exec, char const* path, char const* const* argv) {
  xputs(s);
  xputs(path);
  xputs("\nargv: ");
  xputs(argv[0]);
  xputs("\n");
  return 0;
}
