#include "xlib_net.h"

char* s = "hi from iprx, dev name: ";

int EXT_ENTRY_RO(iprx, uint8_t const* dgram, size_t dlen, struct netdev const* dev) {
  xputs(s);
  xputs(dev->name);
  xputs("\n");
  return 0;
}
