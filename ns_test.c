#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void)
{
  printf(1, "hello from namespace program!\n");

  // What is my namespace?
  int nsid = getnsid();
  printf(1, "namespace: %d\n", nsid);

  // Make a new namespace
  int new_nsid = mkns();
  printf(1, "new namespace: %d\n", new_nsid);

  // Change to the new namepsace
  int chns_status = chns(new_nsid);
  printf(1, "chns_status: %d\n", chns_status);

  // And confirm that the namespace change happened
  int my_new_nsid = getnsid();
  printf(1, "my new namespace: %d\n", my_new_nsid);

  // Attempt to delete the namespace (this should fail);
  int rmns_status = rmns(new_nsid);
  printf(1, "status of rmns attempt: %d\n", rmns_status);

  // Change the namepace
  chns_status = chns(nsid);
  printf(1, "chns_status: %d\n", chns_status);

  // And then delete the namespace
  rmns_status = rmns(new_nsid);
  printf(1, "status of rmns attempt: %d\n", rmns_status);

  exit();
}

