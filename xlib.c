#include "xlib.h"
#include "xtable.h"

typedef void(printf_t)(char const*, ...);

char* kalloc(void) {
  return ((typeof(kalloc)*)((function_t*)XTABLE_ADDR)[0])();
}

void kfree(char*v) {
  ((typeof(kfree)*)((function_t*)XTABLE_ADDR)[1])(v);
}

void xputs(char const* s) {
  ((printf_t*)((function_t*)XTABLE_ADDR)[2])(s);
}

void xputd(int i) {
  ((printf_t*)((function_t*)XTABLE_ADDR)[2])("%d", i);
}

void xputx(int i) {
  ((printf_t*)((function_t*)XTABLE_ADDR)[2])("%x", i);
}

int mypid() {
  return ((typeof(mypid)*)((function_t*)XTABLE_ADDR)[3])();
}
