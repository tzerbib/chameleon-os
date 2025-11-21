#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void)
{
  printf(1, "hello from namespace program!\n");
  int nsid = getnsid();
  printf(1, "namespace: %d\n", nsid);
  exit();
}

