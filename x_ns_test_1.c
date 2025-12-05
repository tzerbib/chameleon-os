#include "types.h"
#include "stat.h"
#include "user.h"

int main(void) {
  printf(1, "hello from x_ns_test_1!\n");
  int nsid = getnsid();
  printf(1, "ns %d\n", nsid);

  int chns_status = chns(1);
  printf(1, "changing to ns 1, status: %d\n", chns_status);

  char* args[] = { "true", 0 };
  exec("true", args);
}
