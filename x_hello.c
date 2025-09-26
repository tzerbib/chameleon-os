// Create a extension user program that defines
//  a extension function f and calls extload and extattach
// An example of how user can load and attach an extension function
// ?? Eventually need support for library functions

#include "types.h"
#include "stat.h"
#include "user.h"


__attribute__((section("extension"))) void* f(void) {
  int a = 4;
  int b = 6;
  return (void*)(a + b);
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
  exit();
}
