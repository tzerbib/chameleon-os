// Create a extension user program that defines
//  a extension function f and calls extload and extattach
// An example of how user can load and attach an extension function
// ?? Eventually need support for library functions

#include "types.h"
#include "stat.h"
#include "user.h"
#include "xtable.h"
#include "xlib.h"


__attribute__((section("extension"))) void* f(void) {
  char* p = ((typeof(kalloc)*)((function_t*)XTABLE_ADDR)[0])();
  p[0] = 'h';
  p[1] = 'i';
  p[2] = ' ';
  p[3] = 'e';
  p[4] = 'x';
  p[5] = 't';
  p[6] = '\n';
  p[7] = '\0';
  ((typeof(xputs)*)((function_t*)XTABLE_ADDR)[2])(p);
  ((typeof(xputs)*)((function_t*)XTABLE_ADDR)[2])("helo\n");
  ((typeof(kfree)*)((function_t*)XTABLE_ADDR)[1])(p);
  return 0;
}

extern unsigned char __start_extension;
extern unsigned char __stop_extension;

int
main(void)
{
  printf(1, "hello from extension!\n");
  struct extension *e;
  extload(&f, &__stop_extension - &__start_extension, &e);
  extattach(e);
  char* args[] = { "true", 0 };
  exec("true", args);
  printf(1, "exiting user main\n");
  exit();
}
