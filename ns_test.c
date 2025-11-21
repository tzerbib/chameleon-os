#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void)
{
  printf(1, "hello from namespace program!\n");
  int nsid = getnsid();
  printf(1, "namespace: %d\n", nsid);
  int new_nsid = mkns();
  printf(1, "new namespace: %d\n", new_nsid);
  int chns_status = chns(new_nsid);
  printf(1, "chns_status: %d\n", chns_status);
  int my_new_nsid = getnsid();
  printf(1, "my new namespace: %d\n", my_new_nsid);
  exit();
}

