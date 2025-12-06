#pragma once
#include "api/hookpoint.h"
enum extstate { EXT_UNUSED, EXT_LOADED, EXT_ATTACHED };

struct extension {
  enum extstate state;         // extension state
  void* (*entry)(void);        // pointer to the extension code
  char name[16];               // extension name (debugging)
  struct namespace* ns;        // pointer to namespace
  enum hookpoint hp;
};

struct extension* ext_load(char* path);
void ext_attach(struct extension* e, enum hookpoint hp);
void ext_detach(struct extension* e);
