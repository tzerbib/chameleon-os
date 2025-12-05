#ifndef XLIB_H
#define XLIB_H

typedef void(*function_t)(void);

char* kalloc(void);
void kfree(char*);
void xputs(char*);
void xputd(int);
int mypid(void);

#endif // XLIB_H
