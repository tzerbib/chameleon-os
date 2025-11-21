#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"

// Gets the namespace for this process
int sys_getnsid(void) {
	struct proc *curproc = myproc();
	return get_nsid(curproc->ns);
}

