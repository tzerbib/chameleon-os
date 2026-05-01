#pragma once

#include "policy/policy.h"
#include "api/hookpoint.h"

enum domain {
    NET,
    SYSCALL,
    SCHED,
    BIO,
};

void policiesinit();

struct policy* get_policy_by_hookpoint(enum hookpoint);
struct policy* get_policy_by_domain(enum domain);
