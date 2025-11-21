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

// Create a namespace
int sys_mkns(void) {
	struct namespace *ns = create_ns();
	if (!ns) {
		cprintf("HERE");
		return -1;
	}
	cprintf("New namespace ptr: %p\n", ns);
	int id = get_nsid(ns);
	return id;
}

// Switch the current proc to given namespace
int sys_chns(void) {
	int nsid;
	if(argint(0, &nsid) < 0) {
		return -1;
	}
	struct proc *curproc = myproc();
	struct namespace *ns = get_ns(nsid);
	if (!ns) {
		return -1;
	}
	return proc_chns(curproc, ns);
}

// Attempt to remove namespace
int sys_rmns(void) {
	int nsid;
	if(argint(0, &nsid) < 0) {
		return -1;
	}
	cprintf("Removing %d\n", nsid);
	struct namespace *ns = get_ns(nsid);
	cprintf("removing namespace ptr: %p\n", ns);
	return destroy_ns(ns);
}
