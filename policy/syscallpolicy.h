#pragma once

#include "policy.h"

struct syscallpolicy {
    struct policy base;
};

void syscallpolicy_ctor(struct syscallpolicy* this);
