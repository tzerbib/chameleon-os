#include "types.h"
#include "stat.h"
#include "user.h"

// Example user program that loads and attaches extensions
int main(void) {
  
  struct extension *e2;
  if (extload("x2.ext", &e2) != 0) {
    return 1;
  }
  extattach(e2);
  printf(1, "x2 attached\n");

  // struct extension *e1;
  // if (extload("x1.ext", &e1) != 0) {
  //   return 1;
  // }
  // extattach(e1);
  // printf(1, "x1 attached\n");

  // char* args[] = { "true", 0 };
  // exec("true", args);

  // struct extension *e3;
  // if (extload("x3.ext", &e3) != 0) {
  //   printf(1, "extload x3 return non-zero\n");
  //   return 1;
  // }
  // printf(1, "loaded x3\n");
  // extattach(e3);
  // printf(1, "after attaching x3\n");

  // char* args[] = { "true", 0 };
  // exec("true", args);

  struct extension *e4;
  if (extload("x4.ext", &e4) != 0) {
    printf(1, "extload x4 return non-zero\n");
    return 1;
  }
  printf(1, "loaded x4\n");
  extattach(e4);
  printf(1, "after attaching x4\n");

  exit();
}
