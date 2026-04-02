#include "user.h"

int main(void) {
  int new_nsid = mkns();
  chns(new_nsid);
  
  struct extension *enet;
  if (extload("xarp.ext", &enet) != 0) {
    return 1;
  }
  extattach(enet);
  printf(1, "xarp attached to %d\n", new_nsid);
  
  int new_nsid2 = mkns();
  chns(new_nsid2);

  struct extension *eip;
  if (extload("xip.ext", &eip) != 0) {
    return 1;
  }
  extattach(eip);
  printf(1, "xip attached to %d\n", new_nsid2);

  exit();
}
