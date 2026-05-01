#pragma once

#include "policy.h"
#include "stack.h"

struct netpolicy {
    struct policy base;
    struct stack* stack;
};

void netpolicy_ctor(struct netpolicy* this);
