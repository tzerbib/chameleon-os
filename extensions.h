#pragma once
#include "api/hookpoint.h"
enum extstate { EXT_UNUSED, EXT_LOADED, EXT_ATTACHED };

struct extension {
  enum extstate state;         // extension state
  void* (*entry)(...);        // pointer to the extension code
  char name[16];               // extension name (debugging)
  struct namespace* ns;        // pointer to namespace
  enum hookpoint hp;
};

struct extension* ext_load(char* path);
void ext_attach(struct extension* e);
void ext_detach(struct extension* e);
enum hookpoint check_hookpoint(char const* name);

#define EXT_HP_NOPS(hp)\
  asm volatile (\
    ".globl sys_"#hp"_nop_start\n"\
    "sys_"#hp"_nop_start:\n"\
    "nop\n"\
    "nop\n"\
    "nop\n"\
    "nop\n"\
    "nop\n"\
    "nop\n"\
    "nop\n"\
    ".globl sys_"#hp"_nop_end\n"\
    "sys_"#hp"_nop_end:\n"\
    : \
    : \
    :);
