#include "api/hookpoint.h"
#include "types.h"
#include "stat.h"
#include "user.h"

int main(void) {
  printf(1, "hello from zombies!\n");

  for (int tries = 0; tries < 10; tries++) {

    struct extension *e;
    extload("x1.ext", &e);
    extattach(e, HP_exec);
  
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
