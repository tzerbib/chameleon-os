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

struct hashmap* hm_alloc(void) {
  return ((typeof(hm_alloc)*)((function_t*)XTABLE_ADDR)[4])();
}

void hm_free(struct hashmap* hm) {
  ((typeof(hm_free)*)((function_t*)XTABLE_ADDR)[5])(hm);
}

int hm_put(struct hashmap* hm, const char* key, void* val) {
  return ((typeof(hm_put)*)((function_t*)XTABLE_ADDR)[6])(hm, key, val);
}

void* hm_get(struct hashmap* hm, const char* key) {
  return ((typeof(hm_get)*)((function_t*)XTABLE_ADDR)[7])(hm, key);
}

int hm_del(struct hashmap* hm, const char* key) {
  return ((typeof(hm_del)*)((function_t*)XTABLE_ADDR)[8])(hm, key);
}

void hm_iter(struct hashmap* hm, hm_iter_fn fn, void* arg) {
  ((typeof(hm_iter)*)((function_t*)XTABLE_ADDR)[9])(hm, fn, arg);
}

struct hashmap* my_maps() {
  return ((typeof(my_maps)*)((function_t*)XTABLE_ADDR)[10])();
}

int mynsid(void) {
  return ((typeof(mynsid)*)((function_t*)XTABLE_ADDR)[11])();
}
