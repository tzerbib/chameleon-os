#include "xlib.h"

char* s = "from x3\nmypid ";

int main(void) {
  if (mypid() == 0) { 
    // Comparing to a dummy value here
    return 0;
  }
  return 1;
}
