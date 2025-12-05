#include "api/hookpoint.h"
#include "types.h"
#include "stat.h"
#include "user.h"

int main(void) {
  int nsid = getnsid();
  printf(1, "ns %d\n", nsid);

  // Attaching x1.ext to ns 0
  struct extension *e;
  extload("x1.ext", &e);
  extattach(e, HP_exec);
  char* args[] = { "true", 0 };

  if (fork() == 0) {
    printf(1, "in child check ns is %d\n", getnsid());
    exec("true", args);
    exit();
  } else {
    int new_nsid = mkns();
    printf(1, "new ns %d\n", new_nsid);
  
    chns(new_nsid);
  
    // Attaching x2.ext to ns 1
    struct extension *e2;
    extload("x2.ext", &e2);
    extattach(e2, HP_exec);

    if (fork() == 0) {
      printf(1, "in child 2 ns is %d\n", getnsid());
      exec("true", args);
      exit();
    } else {
      chns(nsid);
    
      if (fork() == 0) {
        printf(1, "in child 3 ns is %d\n", getnsid());
        exec("true", args);
        exit();
      } else {
        exit();
      }
    }
  }
}
