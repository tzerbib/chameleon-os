#include "types.h"
#include "stat.h"
#include "user.h"

int main(void) {
  char* args[] = { "udpechoserver", 0 };

  if (fork() == 0) {
    chns(1);
  } else {
    chns(2);
  }

  exec("udpechoserver", args);
}
