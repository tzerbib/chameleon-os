#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void)
{
  printf(1, "hello from x_hello_elf!\n");
  struct extension *e;
  extload_elf("x.ext", &e);
  extattach(e);
  char* args[] = { "true", 0 };
  exec("true", args);
  printf(1, "exiting user main\n");
  exit();
}
