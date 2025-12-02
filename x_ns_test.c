#include "types.h"
#include "stat.h"
#include "user.h"

int main(void) {
  printf(1, "hello from x_ns_test!\n");
  int nsid = getnsid();
  printf(1, "ns %d\n", nsid);

  // Attaching x1.ext to ns 0
  struct extension *e;
  extload_elf("x1.ext", &e);
  extattach(e);
  char* args[] = { "true", 0 };

  if (fork() == 0) {
    printf(1, "in child check ns is %d\n", getnsid());
    exec("true", args);
    exit();
  } else {
    int new_nsid = mkns();
    printf(1, "new ns %d\n", new_nsid);
  
    int chns_status = chns(new_nsid);
    printf(1, "changing to new ns, chns_status: %d\n", chns_status);
  
    // Attaching x2.ext to ns 1
    struct extension *e2;
    extload_elf("x2.ext", &e2);
    extattach(e2);

    if (fork() == 0) {
      printf(1, "in child 2 ns is %d\n", getnsid());
      exec("true", args);
      exit();
    } else {
      int chns_status2 = chns(nsid);
      printf(1, "changing back to old ns %d: %d\n", nsid, chns_status2);
    
      if (fork() == 0) {
        printf(1, "in child 3 ns is %d\n", getnsid());
        exec("true", args);
        exit();
      } else {

        printf(1, "exiting user main\n");
        exit();
      }
    }
  }
}
