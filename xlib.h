#ifndef XLIB_H
#define XLIB_H

typedef void(*function_t)(void);

char* kalloc(void);
void kfree(char*);
void xputs(char*);

#endif // XLIB_H
