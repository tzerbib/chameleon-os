#include "xlib.h"
#include "xtable.h"

typedef void(printf_t)(char*, ...);

char* kalloc(void) {
  return ((typeof(kalloc)*)((function_t*)XTABLE_ADDR)[0])();
}

void kfree(char*v) {
  ((typeof(kfree)*)((function_t*)XTABLE_ADDR)[1])(v);
}

void xputs(char * s) {
  ((printf_t*)((function_t*)XTABLE_ADDR)[2])(s);
}

void xputd(int i) {
  ((printf_t*)((function_t*)XTABLE_ADDR)[2])("%d", i);
}

int mypid() {
  return ((typeof(mypid)*)((function_t*)XTABLE_ADDR)[3])();
}
