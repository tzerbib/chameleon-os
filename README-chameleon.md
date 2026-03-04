# Extension Installation in Chameleon

This is an incomplete README. I will make this better!

## Writing

Example extensions:

- `x1.ext.c` (read-write extension for `exec`)
- `x2.ext.c` (read-only extension for `exec`)
- `x3.ext.c` (read-only extension for `swtch`)

## Loading and Attaching

An example user program that loads and attaches extensions:

- `x_hello.c`
  - Loads and attaches x2 then x1, so when `exec` is triggered, it will print the original program to execute and execute the `ls` program.

## Adding New Hookpoints

- Currently supported hookpoints:
  - `exec` (hookpoint at `exec`, `sys_exec` core implementation, in `exec.c`)
  - `getpid` (hookpoint at `sys_getpid` in `sysproc.c`)
  - `read` (hookpoint at `fileread`, `sys_read` core implementation, in `file.c`)
- `api/hookpoint.h` and `hookpoint.h` both define hookpoint enums used all over the code (I might move the former file, it was added before some redesign)
- `hookpoints.c` defines some labels needed for each hookpoint and a `hptable` for easy access to these labels

- `xlib.h` contains corresponding extension entry function signatures---notice the two different MACROs for read/write vs read-only extensions.

## Exposing Helper Functions to Extension

The `xtable` in `extensions` contains pointers to helper functions exposed for extensions to use.
