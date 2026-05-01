#pragma once

enum hookpoint {
  HP_getpid,
  HP_exec,
  HP_read,
  HP_swtch,
  HP_mkdir,
  HP_arprx,
  HP_iprx,
  HP_iptx,
  HP_udprx,
  HP_bread,
  HP_none,
};
