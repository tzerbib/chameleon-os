#include "stack.h"
#include "defs.h"

void stack_init(struct stack* s, char* lk) {
  initlock(&s->lock, lk);
  s->top = &s->arr[0];
}

static int stack_is_empty(struct stack* s) {
  return s->top == &s->arr[0];
}

static int stack_is_full(struct stack* s) {
  return s->top == &s->arr[MAX_STACK_SIZE];
}

int stack_push(struct stack* s, uint16_t e) {
  acquire(&s->lock);
  if (stack_is_full(s)) {
    release(&s->lock);
    return -1;
  }

  *s->top++ = e;
  release(&s->lock);
  return 0;
}

// TODO: update locking of various stack function?
uint16_t stack_pop(struct stack* s) {
  acquire(&s->lock);
  if (stack_is_empty(s)) {
    release(&s->lock);
    panic("pop from empty stack");
  }
  
  uint16_t old_top = *--s->top;
  release(&s->lock);
  return old_top;
}

uint16_t stack_top(struct stack* s) {
  acquire(&s->lock);
  if (stack_is_empty(s)) {
    release(&s->lock);
    panic("pop from empty stack");
  }
  
  uint16_t top = *(s->top - 1);
  release(&s->lock);
  return top;
}
