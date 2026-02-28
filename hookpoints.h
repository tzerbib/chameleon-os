#pragma once
#include "api/hookpoint.h" // IWYU pragma: keep

struct hpt_entry {
  unsigned char* start;
  unsigned char* end;
};

extern struct hpt_entry hptable[];
