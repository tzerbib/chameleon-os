#include "xlib_net.h"

char* s = "hi from arprx, dev name: ";

int EXT_ENTRY_RO(arprx, unsigned char const* packet, unsigned int plen, struct netdev const* dev) {
  xputs(s);
  xputs(dev->name);
  xputs("\n");
  return 0;
}
