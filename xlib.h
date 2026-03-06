#ifndef XLIB_H
#define XLIB_H

typedef void(*function_t)(void);

char* kalloc(void);
void kfree(char*);
void xputs(char const*);
void xputd(int);
void xputx(int);
int mypid(void);

#define EXT_ENTRY_RO(name, args...) \
____roext_at_##name(args)

#define EXT_ENTRY_RW(name, args...) \
____rwext_at_##name(args)

struct context {
  unsigned int edi;
  unsigned int esi;
  unsigned int ebx;
  unsigned int ebp;
  unsigned int eip;
};

// Valid hookpoints
// Read-only
int EXT_ENTRY_RO(getpid, void);
int EXT_ENTRY_RO(exec, char const* path, char const* const* argv);
int EXT_ENTRY_RO(swtch, struct context const* const*, struct context const*);

// Read/write
int EXT_ENTRY_RW(exec, char* path, char** argv);
int EXT_ENTRY_RW(mkdir, char* path, char** argv);

#endif // XLIB_H
