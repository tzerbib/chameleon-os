#include "xlib_net.h"

char* s = "hi from iptx, dst: ";

int EXT_ENTRY_RO(iptx, struct netif const* netif, uint8_t protocol, uint8_t const* buf, size_t len, ip_addr_t const* dst) {
  xputs(s);
  xputx((int)(*dst));
  xputs("\n");
  return 0;
}
