#pragma once
#include "types.h"

#define __CONCAT(a, b) a ## b
#define CONCATENATE(a, b) __CONCAT(a, b)

#define MAX_STACK_SIZE (1 << 12) - 1 // Number of possible VLAN ids

// This is now only a uint16_t stack
// TODO: use macro to template this??
struct stack {
  uint16_t arr[MAX_STACK_SIZE];
  uint16_t* top;
};

void stack_init(struct stack* s);
[[nodiscard]] int stack_is_empty(struct stack const* s);
[[nodiscard]] int stack_push(struct stack* s, uint16_t e);
uint16_t stack_pop(struct stack* s);
[[nodiscard]] uint16_t stack_top(struct stack const* s);
