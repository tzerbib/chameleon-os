#include "xlib.h"
#include "xlib_net.h"

char* s = "hi from udprx, dev name: ";
char* INNER_MAP_NAME = "udprx_counts";


int EXT_ENTRY_RO(udprx, uint8_t const* buf, size_t len, ip_addr_t const* src, ip_addr_t const* dst, struct netif const* iface) {
  xputs(s);
  xputs(iface->dev->name);
  xputs("\n");

  struct hashmap *maps = my_maps(); 
  struct hashmap *udprx_counts = (struct hashmap*)hm_get(maps, INNER_MAP_NAME);
  if (udprx_counts == 0) {
    udprx_counts = hm_alloc();
    if (udprx_counts == 0){
      return 0;
    }
    hm_put(maps, INNER_MAP_NAME, udprx_counts);
  }

  int count = (int)hm_get(udprx_counts, (char*)src); 

  hm_put(udprx_counts, (char*)src, (void*)(count+1));

  xputs("packet from ");
  xputx((int)src);
  xputs(", count: ");
  xputd((int)hm_get(udprx_counts, (char*)src));
  xputs("\n");
  return 0;
}




