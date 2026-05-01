#include "minidisas.h"

#define MOV 0x89
#define MODRM_MR_EAX_EBP 0x45

decode_result_t decode_mov_ebp_eax(uint8_t const* mc) {
    decode_result_t res = { false, 0 };

    uint8_t opcode = mc[0];
    uint8_t modrm  = mc[1];
    int8_t  disp   = (int8_t)mc[2]; // signed!

    if (opcode != MOV) {
      return res;
    }

    // Check ModRM:
    // 01 000 101 = 0x45
    // mod = 01 (disp8)
    // reg = 000 (eax)
    // r/m = 101 (ebp)
    if (modrm != MODRM_MR_EAX_EBP) {
      return res;
    }

    res.valid = true;
    res.offset = disp;

    return res;
}
