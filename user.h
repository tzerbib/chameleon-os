#include "api/hookpoint.h"
#include "types.h"

struct stat;
struct rtcdate;
struct extension;

// system calls
int fork(void);
int exit(void) __attribute__((noreturn));
int wait(void);
int pipe(int*);
int write(int, const void*, int);
int read(int, void*, int);
int close(int);
int kill(int);
int exec(char*, char**);
int open(const char*, int);
int mknod(const char*, short, short);
int unlink(const char*);
int fstat(int fd, struct stat*);
int link(const char*, const char*);
int mkdir(const char*);
int chdir(const char*);
int dup(int);
int getpid(void);
char* sbrk(int);
int sleep(int);
int uptime(void);
int extattach(struct extension*, enum hookpoint);
int extload(char* p, struct extension**);
int getnsid(void);
int mkns(void);
int chns(int);
int rmns(int);

// ulib.c
int stat(const char*, struct stat*);
char* strcpy(char*, const char*);
void *memmove(void*, const void*, int);
char* strchr(const char*, char c);
int strcmp(const char*, const char*);
void printf(int, const char*, ...);
char* gets(char*, int max);
uint strlen(const char*);
void* memset(void*, int, uint);
void* malloc(uint);
void free(void*);
int atoi(const char*);

static inline uint64 rdtsc(void) {
  uint cycles_high;
  uint cycles_low;
  asm volatile (
    "rdtsc\n"
    "mov %%edx, %0\n"
    "mov %%eax, %1\n"
    : "=r" (cycles_high), "=r" (cycles_low)
    :
    :"%rax", "rbx", "rcx", "rdx");
  uint64 cycles = cycles_high;
  cycles <<= 32;
  cycles |= cycles_low;
  return cycles;
}
