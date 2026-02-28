.macro nops_for hp
  .globl sys_\hp\()_nop_start
  sys_\hp\()_nop_start:
  nop
  nop
  nop
  nop
  nop
  nop
  nop
  .globl sys_\hp\()_nop_end
  sys_\hp\()_nop_end:
.endm
