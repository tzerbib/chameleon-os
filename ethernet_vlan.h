#pragma once

#include "ethernet.h"
#include "defs.h"
#include "stack.h"

#define ETHERNET_HDR_SIZE_VLAN 18
#define ETHERNET_FRAME_SIZE_MIN_VLAN 68
#define ETHERNET_FRAME_SIZE_MAX_VLAN 1522

#define VLAN_IEEE802_1Q_TPID 0x8100
#define VID_BITMASK ((1 << 12) - 1)
#define PCP_SHIFT 9

// TODO: Follwoing may be moved to extension.h?
extern struct stack vlan_stack;

void vlanstackinit();

struct vlan_hdr {
  uint16_t tpid;
  uint16_t pcp : 3;
  uint16_t dei : 1;
  uint16_t vid : 12;
};

struct ethernet_hdr_vlan {
  uint8_t dst[ETHERNET_ADDR_LEN];
  uint8_t src[ETHERNET_ADDR_LEN];
  struct vlan_hdr vlan;
  uint16_t type;
};

int is_vlan_tagged(uint8_t const* frame);
void ethernet_vlan_dump(struct netdev *dev, uint8_t const *frame, size_t flen);
uint8_t* shift_hdr_head(uint8_t* frame);
uint16_t get_vid(uint8_t const *frame);

ssize_t ethernet_vlan_tx_helper(struct netdev *dev, uint16_t type, const uint8_t *payload, size_t plen, const void *dst, ssize_t (*cb)(struct netdev*, uint8_t*, size_t));
