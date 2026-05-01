#ifndef XLIB_NET_H
#define XLIB_NET_H

#include "xlib.h"
#define IFNAMSIZ 16

typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef uint32_t size_t;
typedef uint32_t ip_addr_t;

struct netif {
    struct netif *next;
    uint8_t family;
    struct netdev *dev;
};

struct netdev {
    struct netdev *next;
    struct netif *ifs;
    int index;
    char name[IFNAMSIZ];
    uint16_t type;
    uint16_t mtu;
    uint16_t flags;
    uint16_t hlen;
    uint16_t alen;
    uint8_t addr[16];
    uint8_t peer[16];
    uint8_t broadcast[16];
    struct netdev_ops *ops;
    void *priv;
};

// Read-only hookpoints
int EXT_ENTRY_RO(arprx, unsigned char const* packet, unsigned int plen, struct netdev const* dev);
int EXT_ENTRY_RO(iprx, uint8_t const* dgram, size_t dlen, struct netdev const* dev);
int EXT_ENTRY_RO(iptx, struct netif const* netif, uint8_t protocol, uint8_t const* buf, size_t len, ip_addr_t const* dst);
int EXT_ENTRY_RO(udprx, uint8_t const* buf, size_t len, ip_addr_t const* src, ip_addr_t const* dst, struct netif const* iface);

#endif // XLIB_NET_H
