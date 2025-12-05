#include "api/hookpoint.h"
#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void)
{
  printf(1, "hello from extension!\n");
  struct extension *e;
  extload("x3.ext", &e);
  extattach(e, HP_getpid);
  printf(1, "getpid %d\n", getpid());
  printf(1, "exiting user main\n");
  exit();
}
