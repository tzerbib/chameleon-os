#include "api/hookpoint.h"
#include "extensions.h"
#include "types.h"
#include "stat.h"
#include "user.h"

int main(void) {
  printf(1, "hello from x_time_1!\n");
  int old_ns = getnsid();

  // Basline
  uint64 start = rdtsc();
  for (int i = 0; i < 10000; i++) {
    getpid();
  }
  uint64 end = rdtsc();
  
  printf(1, "[0] duration is %z (nops only)\n", end - start);

  int new_ns = mkns();

  for (int i = 0; i < 12; i++) {
    // Attaching to old ns
    struct extension* e;
    extload("x3.ext", &e);
    extattach(e, HP_getpid);
    
    start = rdtsc();
    for (int i = 0; i < 10000; i++) {
      getpid();
    }
    end = rdtsc();

    printf(1, "[%d] duration is %z (trampoline + ext)\n", i + 1, end - start);

    chns(new_ns);
    start = rdtsc();
    for (int i = 0; i < 10000; i++) {
      getpid();
    }
    end = rdtsc();

    printf(1, "[%d] duration is %z (trampoline)\n", i + 1, end - start);

    chns(old_ns);
  }

  exit();
}
