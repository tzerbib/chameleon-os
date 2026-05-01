#include "user.h"

int main(void) {
  // if (fork() == 0) {
  //     printf(1, "ifconfig command 1 in ns %d\n", getnsid());
  //     char* args[] = { "ifconfig", "net1", "172.16.101.2/24", 0 };
  //     printf(1, "in child check ns is %d\n", getnsid());
  //     exec("ifconfig", args);
  //     exit();
  // }

  // wait();

  mkns();
  mkns();
  
  // struct extension *eudprx;
  // if (extload("udprx.ext", &eudprx) != 0) {
  //   return 1;
  // }
  // extattach(eudprx);
  // printf(1, "udprx.ext attached to %d\n", getnsid());
  
  // struct extension *eiptx;
  // if (extload("iptx.ext", &eiptx) != 0) {
  //   return 1;
  // }
  // extattach(eiptx);
  // printf(1, "iptx.ext attached to %d\n", getnsid());
  // struct extension *eiprx;
  // if (extload("iprx.ext", &eiprx) != 0) {
  //   return 1;
  // }
  // extattach(eiprx);
  // printf(1, "iprx.ext attached to %d\n", getnsid());
  
  chns(2);

  // struct extension *eiptx;
  // if (extload("iptx.ext", &eiptx) != 0) {
  //   return 1;
  // }
  // extattach(eiptx);
  // printf(1, "iptx.ext attached to %d\n", getnsid());
  // struct extension *earprx;
  // if (extload("arprx.ext", &earprx) != 0) {
  //   return 1;
  // }
  // extattach(earprx);
  // printf(1, "arprx.ext attached to %d\n", getnsid());

  struct extension *eiprx2;
  if (extload("iprx.ext", &eiprx2) != 0) {
    return 1;
  }
  extattach(eiprx2);
  printf(1, "iprx.ext attached to %d\n", getnsid());

  exit();
}
