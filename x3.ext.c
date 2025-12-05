#include "xlib.h"

int main(void) {
  if (mypid() == 0) { 
    // Comparing to a dummy value here
    return 0;
  }
  return 1;
}
