#include "extcontexts.h"
#include "api/hookpoint.h"
#include "extension/biocontext.h"
#include "extension/extcontext.h"
#include "defs.h"

// TODO: have the Makefile pass in an opt level?
int get_optimization_level() {
  #ifdef __OPTIMIZE__
      return 2;
  #else
      return 0;
  #endif
}

static build_context_t* build_contexts_o0[] = {
  [HP_bread] = &build_context_bread,
  [HP_getpid] = nullptr,
  [HP_read] = nullptr,
  [HP_exec] = nullptr,
  [HP_swtch] = nullptr,
  [HP_mkdir] = nullptr,
  [HP_arprx] = nullptr,
  [HP_iprx] = nullptr,
  [HP_iptx] = nullptr,
  [HP_udprx] = nullptr,
  [HP_bflush] = &build_context_bflush,
};

// TODO: none of the following is implemented
static build_context_t* build_contexts_o2[] = {
  [HP_bread] = nullptr,
  [HP_getpid] = nullptr,
  [HP_read] = nullptr,
  [HP_exec] = nullptr,
  [HP_swtch] = nullptr,
  [HP_mkdir] = nullptr,
  [HP_arprx] = nullptr,
  [HP_iprx] = nullptr,
  [HP_iptx] = nullptr,
  [HP_udprx] = nullptr,
  [HP_bflush] = nullptr,
};

build_context_t* get_build_context_f(enum hookpoint hp) {
  if (get_optimization_level() == 0) {
    return build_contexts_o0[hp];
  }
  return build_contexts_o2[hp];
}
