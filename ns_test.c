#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

void test_ns_read(void){
  char buf0[512];
  char buf1[512];
  int pid;

  printf(1, "\nSTARTING TEST_NS_READ\n");

  printf(1, "in ns 0\n");

  // read something as nsid 0
  int fd = open("README", O_RDONLY);
  if(fd < 0){
      printf(1, "failed to open README\n");
      return;
  }
  int bytes_read0 = read(fd, buf0, sizeof(buf0));
  close(fd);
  printf(1, "ns 0 read %d bytes\n", bytes_read0);

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
      printf(1, "namespace didn't change!!\n");
      return;
  }

  // try to open a file in the new ns 
  fd = open("README", O_RDONLY);
  if(fd < 0){
      printf(1, "ns %d cannot open readme\n", getnsid());
      return;
  } 
  int bytes_read1 = read(fd, buf1, sizeof(buf1));
  close(fd);

  printf(1, "ns %d read %d bytes\n", getnsid(), bytes_read1);

  // printf(1, "buf0 == buf1: %d", strcmp(buf0, buf1) == 0); 
  // assert(strcmp(buf0, buf1) == 0);

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