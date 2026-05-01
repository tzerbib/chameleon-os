#include "api/hookpoint.h"
#include "hookpoints.h"
#include "types.h"
#include "defs.h"
#include "mmu.h"
#include "param.h"
#include "proc.h"
#include "namespace.h"
#include "extensions.h"
#include "trampoline.h"
#include "ethernet_vlan.h"


uint check_arity(enum hookpoint hp) {
  switch (hp) {
  case HP_getpid:
    return 0;
  case HP_swtch:
  case HP_exec:
    return 2;
  case HP_arprx:
  case HP_iprx:
  case HP_read:
    return 3;
  case HP_mkdir: 
  case HP_bread:
    return 1;
  case HP_iptx:
  case HP_udprx:
    return 5;
  default:
    panic("Called check_arity with HP_none");
  }
}

#define TRAMPOLINE_CALL_LABELS(arity)\
  extern unsigned char trampoline##arity##_call_start;\
  extern unsigned char trampoline##arity##_call_end; 

#define TRAMPOLINE_CALL(arity)\
  asm volatile (\
    "jmp trampoline" #arity "_call_end\n"\
    ".globl trampoline" #arity "_call_start\n"\
    "trampoline" #arity "_call_start:\n"\
    "push %%ebp\n"\
    "call trampoline" #arity "\n"\
    "pop %%ebp\n"\
    ".globl trampoline" #arity "_call_end\n"\
    "trampoline" #arity "_call_end:\n"\
    :\
    :\
    : "rax", "rcx", "rdx", "cc", "memory");

#define TRAMPOLINE_CASE(arity)\
  case arity:\
  *tcp = (struct trampoline_call) {\
    .start = &trampoline##arity##_call_start,\
    .end = &trampoline##arity##_call_end,\
  };\
  break;

// Adapted from Linux source code
#define COUNT_ARGS_HELPER(_0, _1, _2, _3, _4, _5, _6, _n, ...) _n
#define COUNT_ARGS(X...) COUNT_ARGS_HELPER(, ##X, 6, 5, 4, 3, 2, 1, 0)

#define UNPACK(XS...) XS
#define REPEAT_0(FN, DL, ...)
#define REPEAT_1(FN, DL, X, ...) FN(X)
#define REPEAT_2(FN, DL, X, XS...) FN(X) UNPACK DL REPEAT_1(FN, DL, XS)
#define REPEAT_3(FN, DL, X, XS...) FN(X) UNPACK DL REPEAT_2(FN, DL, XS)
#define REPEAT_4(FN, DL, X, XS...) FN(X) UNPACK DL REPEAT_3(FN, DL, XS)
#define REPEAT_5(FN, DL, X, XS...) FN(X) UNPACK DL REPEAT_4(FN, DL, XS)
#define REPEAT_6(FN, DL, X, XS...) FN(X) UNPACK DL REPEAT_5(FN, DL, XS)
#define REPEAT_7(FN, DL, X, XS...) FN(X) UNPACK DL REPEAT_6(FN, DL, XS)
#define REPEAT(N, FN, DL, XS...) REPEAT_##N(FN, DL, XS)
#define ALSO_REPEAT_0(FN, DL, ...)
#define ALSO_REPEAT_1(FN, DL, X, ...) FN(X)
#define ALSO_REPEAT_2(FN, DL, X, XS...) FN(X) UNPACK DL ALSO_REPEAT_1(FN, DL, XS)
#define ALSO_REPEAT_3(FN, DL, X, XS...) FN(X) UNPACK DL ALSO_REPEAT_2(FN, DL, XS)
#define ALSO_REPEAT_4(FN, DL, X, XS...) FN(X) UNPACK DL ALSO_REPEAT_3(FN, DL, XS)
#define ALSO_REPEAT_5(FN, DL, X, XS...) FN(X) UNPACK DL ALSO_REPEAT_4(FN, DL, XS)
#define ALSO_REPEAT_6(FN, DL, X, XS...) FN(X) UNPACK DL ALSO_REPEAT_5(FN, DL, XS)
#define ALSO_REPEAT_7(FN, DL, X, XS...) FN(X) UNPACK DL ALSO_REPEAT_6(FN, DL, XS)
#define ALSO_REPEAT(N, FN, DL, XS...) ALSO_REPEAT_##N(FN, DL, XS)

#define EXPAND(XS...) XS

#define EBP_ARG(X) ebp[(X)+2]

#define DL_COM (,)
#define DL_NL ()
#define SEQ_0_7	0, 1, 2, 3, 4, 5, 6

#define CALL_ENTRY
#define BUILD_ARGS_GENERIC(arity)\
  e->entry(REPEAT(arity, EBP_ARG, DL_COM, SEQ_0_7))

enum hookpoint find_hp_by_addr(unsigned char const* addr) {
    for (enum hookpoint index = 0; index < HP_none; index++) {
      struct hpt_entry* entry = hptable + index;
      if (entry->start <= addr && addr < entry->end) {
        return index;
      }
  }
  panic("no hp found by addr");
}

// TODO: in progress...?
struct context_exec {
  char const* path;
  char const* const* args;
};

static void build_context_exec(uint const* ebp, struct context_exec* ctx) {
  ctx->path = (typeof(ctx->path)) ebp[2];
  ctx->args = (typeof(ctx->args)) ebp[3];
}

/*
 * create a context object
 * populate it once from ebp using build_context
 * pass its address to e->entry
 */

#define TRAMPOLINE(arity)\
  void trampoline##arity(uint const* ebp [[maybe_unused]]) {\
    struct proc *currproc = myproc();\
    struct namespace *currns;\
    if (currproc == 0) {\
      /* TODO: the ns is dependent on vid or something else */\
      uint16_t vid = stack_top(&vlan_stack);\
      /* TODO: maybe change nsid to uint16 */\
      currns = get_ns((int)vid);\
      if (currns == 0) {\
        /* No ns created for the vid*/\
        return;\
      }\
    } else {\
      currns = currproc->ns;\
    }\
    struct ns_object* ns_obj;\
    unsigned char* ret_addr;\
    asm volatile (\
      "movl 4(%%ebp), %0"\
      : "=r"(ret_addr)\
      :\
      :);\
    enum hookpoint trigger = find_hp_by_addr(ret_addr);\
    /* TODO: figure out how to remove locking */\
    /* acquire(&currns->lock); */\
    /* TODO: Build context here depending on trigger */\
    for (ns_obj = currns->namespaced_exts; ns_obj < &currns->namespaced_exts[N_NS_EXT]; ++ns_obj) {\
      void* p = ns_obj->pointer;\
      if (ns_obj->kind == NONE) {\
        break;\
      }\
      struct extension* e = (struct extension*)p;\
      /* TODO: conditions here need rethinking for detachment */\
      if (e->state != EXT_ATTACHED) {\
        break;\
      }\
      if (e->hp != trigger) {\
        continue;\
      }\
      EXPAND(BUILD_ARGS_GENERIC(arity));\
    }\
    /* release(&currns->lock); */\
  }

ALSO_REPEAT(7, TRAMPOLINE, DL_NL, SEQ_0_7)

void trampoline_call_for(enum hookpoint hp, struct trampoline_call* tcp) {
  REPEAT(7, TRAMPOLINE_CALL_LABELS, DL_NL, SEQ_0_7)

  REPEAT(7, TRAMPOLINE_CALL, DL_NL, SEQ_0_7)

  switch (check_arity(hp)) {
  REPEAT(7, TRAMPOLINE_CASE, DL_NL, SEQ_0_7)
  default:
    panic("Not a valid arity");
  }
}

