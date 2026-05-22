#include "policies.h"
#include "api/hookpoint.h"
#include "policy/biopolicy.h"
#include "policy/netpolicy.h"
#include "policy/policy.h"
#include "policy/syscallpolicy.h"

#define NEW(T, args...)\
    ({\
        static struct T name;\
        T##_ctor(&name, ##args);\
        &name;\
    })

static enum domain domains[] = {
    [HP_getpid] = SYSCALL,
    [HP_read] = SYSCALL,
    [HP_exec] = SYSCALL,
    [HP_swtch] = SCHED,
    [HP_mkdir] = SYSCALL,
    [HP_arprx] = NET,
    [HP_iprx] = NET,
    [HP_iptx] = NET,
    [HP_udprx] = NET,
    [HP_bread] = BIO,
    [HP_bflush] BIO,
};

static struct policy* policies[] = {
    [NET] = nullptr,
    [SYSCALL] = nullptr,
    [BIO] = nullptr,
    [SCHED] = nullptr,
};


void policiesinit() {
  policies[NET] = (struct policy*) NEW(netpolicy);
  policies[SYSCALL] = (struct policy*) NEW(syscallpolicy);
  policies[BIO] = (struct policy*) NEW(biopolicy);
}

struct policy* get_policy_by_hookpoint(enum hookpoint hp) {
    enum domain domain = domains[hp];
    return policies[domain];
}

struct policy* get_policy_by_domain(enum domain domain) {
    return policies[domain];
}
