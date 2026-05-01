#pragma once
#include "types.h"

typedef struct {
    bool valid;
    int8_t offset; // signed offset from ebp
} decode_result_t;

decode_result_t decode_mov_ebp_eax(uint8_t const* mc);
