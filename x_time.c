#include "api/hookpoint.h"
#include "types.h"
#include "stat.h"
#include "user.h"

int main(void) {
  printf(1, "hello from x_time!\n");
  printf(1, "uptime: %d\n", uptime());
  uint64 cycles = rdtsc();
  printf(1, "rdtsc 0x%z\n", cycles);

  uint64 start = rdtsc();
  for (int i = 0; i < 100000; i++) {
    getpid();
  }
  uint64 end = rdtsc();
  
  printf(1, "[0] duration is %z\n", end - start);
  
  struct extension* e;
  extload("x3.ext", &e);
  extattach(e, HP_getpid);
  
  start = rdtsc();
  for (int i = 0; i < 100000; i++) {
    getpid();
  }
  end = rdtsc();
  printf(1, "[1] duration is %z\n", end - start);
  
  int new_ns = mkns();
  chns(new_ns);

  start = rdtsc();
  for (int i = 0; i < 100000; i++) {
    getpid();
  }
  end = rdtsc();
  printf(1, "[2] duration is %z\n", end - start);

  exit();
}
