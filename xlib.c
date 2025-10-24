#include "xlib.h"
#include "xtable.h"


char* kalloc(void) {
  return ((typeof(kalloc)*)((function_t*)XTABLE_ADDR)[0])();
}

void kfree(char*v) {
  ((typeof(kfree)*)((function_t*)XTABLE_ADDR)[1])(v);
}

void xputs(char * s) {
  // char* p = kalloc();
  
  ((typeof(xputs)*)((function_t*)XTABLE_ADDR)[2])(s);

  ((typeof(xputs)*)((function_t*)XTABLE_ADDR)[2])("xputs\n");
  // kfree(p);
}
