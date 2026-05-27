#include "biocontext.h"
#include "api/hookpoint.h"
#include "defs.h"
#include "extension/extcontext.h"
#include "types.h"
#include "minidisas.h"
#include "proc.h"

#define OFFSET_TO_MOV_BREAD 0x1b

// This only works under -O0
// This also assumes that the value grabbed is a ptr?
int build_context_bread(uint const* ebp, uint32_t ret_addr, struct extcontext* ctx) {
  uint8_t* mov_addr = (uint8_t*)(ret_addr - OFFSET_TO_MOV_BREAD);
  decode_result_t res = decode_mov_ebp_eax(mov_addr);
  if (!res.valid) {
    panic("Not able to build context for bread"); // TODO: remove panic?
    return -1;
  }
  uint32_t* b_ptr = (uint32_t*)((uint32_t)ebp + res.offset);
  struct buf* b = (struct buf*)(*b_ptr);
  ctx->domain = BIO;
  ctx->bio.buf = b;
  return 0;
}

#define OFFSET_TO_MOV_BFLUSH 0x22


int build_context_bflush(uint const* ebp, uint32_t ret_addr, struct extcontext* ctx) {
 //cprintf("ret_addr = %x\n", ret_addr);
  uint8_t* mov_addr = (uint8_t*)(ret_addr - OFFSET_TO_MOV_BFLUSH);

  // decode_result_t res = decode_mov_ebp_eax(mov_addr);

  // if (!res.valid) {  
  //   panic("Not able to build context for bflush"); // TODO: remove panic?
  //   return -1;
  // }

  int8_t offset = (int8_t)mov_addr[2];
  uint32_t* b_ptr = (uint32_t*)((uint8_t*)ebp + offset);

  struct buf* b = (struct buf*)(*b_ptr);
  ctx->domain = BIO;
  ctx->bio.buf = b;

  assert(b->next != 0 && b->prev != 0 && b->next->prev == b && b->prev->next == b);

  return 0;
}
