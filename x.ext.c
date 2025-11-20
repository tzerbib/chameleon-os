#include "xlib.h"

char c = 'g';
char* s = "from data\n";
void f(void) {
  
  char* p = kalloc();
  p[0] = 'h';
  p[1] = 'i';
  p[2] = ' ';
  p[3] = c;
  p[4] = '\n';
  p[5] = '\0';
  xputs(p);

  xputs(s);
  
  kfree(p);
}


int
main(void)
{
  char* p = kalloc();
  p[0] = 'h';
  p[1] = 'i';
  p[2] = ' ';
  p[3] = 'e';
  p[4] = 'x';
  p[5] = 't';
  p[6] = '\n';
  p[7] = '\0';
  xputs(p);
  f();
  
  kfree(p);
}
