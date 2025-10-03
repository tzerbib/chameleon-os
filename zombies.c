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
  printf(1, "hello from zombies!\n");

  for (int tries = 0; tries < 10; tries++) {

    struct extension *e;
    extload(&f, &__stop_extension - &__start_extension, &e);
    extattach(e);
  
    char* args[] = { "true", 0 };
  
    for (int i = 0; i < 1000; i++) {
      if (fork() == 0) {
        if (exec("true", args) < 0) {
            printf(1, "0 true failed at i %d!\n", i);
            // break;
            exit();
        }
      }
      wait();
    }
  }

  printf(1, "exiting zombies\n");
  exit();
}
