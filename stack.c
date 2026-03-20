#include "stack.h"
#include "defs.h"
#include "types.h"

void stack_init(struct stack *s) {
  s->top = &s->arr[0];
}

int stack_is_empty(struct stack const* s) {
  return s->top == &s->arr[0];
}

static int stack_is_full(struct stack const* s) {
  return s->top == &s->arr[MAX_STACK_SIZE];
}

int stack_push(struct stack *s, uint16_t e) {
  if (stack_is_full(s)) {
    return -1;
  }

  *s->top++ = e;
  return 0;
}

uint16_t stack_pop(struct stack* s) {
  if (stack_is_empty(s)) {
    panic("pop from empty stack");
  }

  return *--s->top;
}

uint16_t stack_top(struct stack const* s) {
  if (stack_is_empty(s)) {
    panic("pop from empty stack");
  }

  return *(s->top - 1);
}
