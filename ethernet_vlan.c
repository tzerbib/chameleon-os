#include "ethernet.h"
#include "ethernet_vlan.h"
#include "defs.h"
#include "net.h"
#include "stack.h"
#include "types.h"
#include "mmu.h"
#include "param.h"
#include "proc.h"

struct stack vlan_stack;

void vlanstackinit() {
    stack_init(&vlan_stack, "vlan_stack");
}

static void vlan_hdr_dump(struct vlan_hdr const* hdr) {
  uint16_t* tci_ptr = (uint16_t*)(hdr) + 1;
  uint16_t tci = ntoh16(*tci_ptr);
  uint16_t pcp = tci >> PCP_SHIFT;
  uint16_t vid = tci & VID_BITMASK;
  cprintf(" vlan:\n");
  cprintf("  tpid: 0x%04x (%s)\n", ntoh16(hdr->tpid), ntoh16(hdr->tpid) == VLAN_IEEE802_1Q_TPID ? "IEEE 802.1Q" : "UNKNOWN");
  cprintf("   pcp: 0b%03b\n", pcp);
  cprintf("   dei: 0b%01b\n", hdr->dei);
  cprintf("   vid: 0x%02x\n", vid);
}

uint16_t get_vid(uint8_t const *frame) {
    if (!is_vlan_tagged(frame)) {
        panic("Frame is not vlan tagged");
    }
    struct ethernet_hdr_vlan *hdr = (struct ethernet_hdr_vlan *)frame;
    uint16_t* tci_ptr = (uint16_t*)(&hdr->vlan) + 1;
    uint16_t tci = ntoh16(*tci_ptr);
    uint16_t vid = tci & VID_BITMASK;
    return vid;
}

void ethernet_vlan_dump(struct netdev *dev, uint8_t const *frame, size_t flen) {
    struct ethernet_hdr_vlan *hdr;
    char addr[ETHERNET_ADDR_STR_LEN];

    hdr = (struct ethernet_hdr_vlan *)frame;
    cprintf("  dev: %s (%s)\n", dev->name, ethernet_addr_ntop(dev->addr, addr, sizeof(addr)));
    cprintf("  src: %s\n", ethernet_addr_ntop(hdr->src, addr, sizeof(addr)));
    cprintf("  dst: %s\n", ethernet_addr_ntop(hdr->dst, addr, sizeof(addr)));
    vlan_hdr_dump(&hdr->vlan);
    cprintf(" type: 0x%04x (%s)\n", ntoh16(hdr->type), ethernet_type_ntoa(hdr->type));
    cprintf("  len: %u octets\n", flen);
    hexdump(frame, flen);
}

int is_vlan_tagged(uint8_t const* frame) {
  struct ethernet_hdr_vlan* hdr = (struct ethernet_hdr_vlan *) frame;
  return ntoh16(hdr->vlan.tpid) == VLAN_IEEE802_1Q_TPID;
}

uint8_t* shift_hdr_head(uint8_t* frame) {
  uint32_t shift_by = sizeof(struct vlan_hdr);
  uint32_t copy_amount = sizeof(struct ethernet_hdr) - sizeof(uint16_t);
  memcpy(frame + shift_by, frame, copy_amount);
  return frame + shift_by;
}

// TODO: this is to just make things work, will need to redesign things!
ssize_t ethernet_vlan_tx_helper(struct netdev *dev, uint16_t type, const uint8_t *payload, size_t plen, const void *dst, ssize_t (*cb)(struct netdev*, uint8_t*, size_t)) {
    uint8_t frame[ETHERNET_FRAME_SIZE_MAX_VLAN];
    struct ethernet_hdr_vlan *hdr;
    size_t flen;

    if (!payload || plen > ETHERNET_PAYLOAD_SIZE_MAX || !dst) {
        return -1;
    }
    memset(frame, 0, sizeof(frame));
    hdr = (struct ethernet_hdr_vlan *)frame;
    memcpy(hdr->dst, dst, ETHERNET_ADDR_LEN);
    memcpy(hdr->src, dev->addr, ETHERNET_ADDR_LEN);
    
    hdr->vlan.tpid = hton16(VLAN_IEEE802_1Q_TPID);

    uint16_t* tci_ptr = (uint16_t*)(&hdr->vlan) + 1;
    // TODO: this assumes pcp and dei are both 0s
    uint16_t vid;
    struct proc *currproc = myproc();
    if (currproc == 0) {
        vid = stack_top(&vlan_stack);
    } else {
        vid = get_nsid(currproc->ns);
    }
#ifdef DEBUG
    cprintf("tx vid: %u\n", vid);
#endif
    *tci_ptr = hton16(vid);
    
    hdr->type = hton16(type);
    memcpy(hdr + 1, payload, plen);
    flen = sizeof(struct ethernet_hdr) + (plen < ETHERNET_PAYLOAD_SIZE_MIN ? ETHERNET_PAYLOAD_SIZE_MIN : plen);
#ifdef DEBUG
    cprintf(">>> ethernet_vlan_tx <<<\n");
    ethernet_vlan_dump(dev, frame, flen);
#endif
    return cb(dev, frame, flen) == (ssize_t)flen ? (ssize_t)plen : -1;
}
