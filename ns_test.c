#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"


void test_ns_read(void){
  char buf[512];
  int pid;

  printf(1, "in ns 0\n");

  // read something as nsid 0
  int fd = open("README", O_RDONLY);
  if(fd < 0){
      printf(1, "failed to open README\n");
      return;
  }
  read(fd, buf, 512);
  close(fd);
  printf(1, "read something in ns 0\n");

  // try to create a new ns
  int nsid = mkns(); 
  if (nsid < 0){
      printf(1, "mkns failed\n"); 
      return; 
  }

  printf(1, "successfully made nsid %d\n", nsid);

  // try to switch the ns
  if (chns(nsid) < 0){
      printf(1, "chns failed\n"); 
      return;
  }

  printf(1, "switched to nsid %d\n", getnsid());

  if (getnsid() != nsid){
      printf(1, "namespace didn't change!!");
      return;
  }

  // try to open a file in the new ns 
    fd = open("README", O_RDONLY);
  if(fd < 0){
      printf(1, "ns %d cannot open readme\n", getnsid());
      return;
  } 
  read(fd, buf, sizeof(buf));
  close(fd);
  printf(1, "ns %d: read something in ns \n", getnsid());
}


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

  test_ns_read();

  exit();
}