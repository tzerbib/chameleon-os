#include "api/hookpoint.h"
#include "types.h"
#include "stat.h"
#include "user.h"

int main(void) {
  printf(1, "hello from x_time!\n");
  int old_ns = getnsid();

  uint64 cycles = rdtsc();
  printf(1, "rdtsc 0x%z\n", cycles);

  // All getpids below should be normal (with nops)
  uint64 start = rdtsc();
  for (int i = 0; i < 100000; i++) {
    getpid();
  }
  uint64 end = rdtsc();
  
  printf(1, "[0] duration is %z (nops only)\n", end - start);
  
  // Attaching x3 to old ns
  struct extension* e;
  extload("x3.ext", &e);
  extattach(e, HP_getpid);

  // TODO: show attaching more extension does not incur more cost with ns
  
  // All getpids below go to the trampoline and the extension
  start = rdtsc();
  for (int i = 0; i < 100000; i++) {
    getpid();
  }
  end = rdtsc();
  printf(1, "[1] duration is %z (trampoline + ext)\n", end - start);
  
  int new_ns = mkns();
  chns(new_ns);

  // All getpids below go to only the trampoline
  start = rdtsc();
  for (int i = 0; i < 100000; i++) {
    getpid();
  }
  end = rdtsc();
  printf(1, "[2] duration is %z (trampoline)\n", end - start);

  chns(old_ns);

  // Detaching e, this will put nops back to getpid
  extdetach(e);

  // All getpids below should be normal (with nops)
  start = rdtsc();
  for (int i = 0; i < 100000; i++) {
    getpid();
  }
  end = rdtsc();
  printf(1, "[3] duration is %z (nops only)\n", end - start);

  exit();
}
