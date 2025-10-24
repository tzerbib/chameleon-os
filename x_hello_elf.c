// Create a extension user program that defines
//  a extension function f and calls extload and extattach
// An example of how user can load and attach an extension function
// ?? Eventually need support for library functions

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
