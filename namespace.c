#include "types.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "namespace.h"
#include "mmu.h"          // so that proc.h gets its requisite types and defs
#include "proc.h"

#define N_NS            16
#define N_NS_OBJ        16

enum slot_state {
	AVAILABLE,
	TAKEN,
};

enum ns_entry_kind {
	EXTENSION,
	PROCESS,
	NONE,
};

struct ns_object {
	void *pointer;
	enum ns_entry_kind kind;
};

struct namespace {
	struct spinlock lock;
	// points to objects in the namespace
	struct ns_object namespaced_objects[N_NS_OBJ];
	enum slot_state slot_state;
};


struct {
	struct spinlock lock;
	struct namespace namespaces[N_NS];
} namespace_table;

struct namespace * create_ns() {

	struct namespace *ns = namespace_table.namespaces;
	struct namespace *end = &namespace_table.namespaces[N_NS];

	for (; ns < end; ++ns) {
		acquire(&ns->lock);
		if (ns->slot_state == AVAILABLE) {
			ns->slot_state = TAKEN;
			release(&ns->lock);
			break;
		}
		release(&ns->lock);
	}

	if (ns == end) {
		return (void *)0x0;
	} else {
		return ns;
	}
}

void initialize_namespace(struct namespace *ns) {
	acquire(&ns->lock);
	ns->slot_state = NONE;
	struct ns_object *ns_obj = ns->namespaced_objects;
	struct ns_object *end = &ns->namespaced_objects[N_NS_OBJ];
	for (; ns_obj < end; ++ns_obj) {
		ns_obj->kind = NONE;
	}
	release(&ns->lock);
}

// Must be called on OS init. initializes the namespace table
int namespaceinit(void) {

	acquire(&namespace_table.lock);

	struct namespace *ns = namespace_table.namespaces;
	acquire(&ns->lock);
	ns->slot_state = TAKEN;
	release(&ns->lock);

	struct namespace *end = &namespace_table.namespaces[N_NS];
	for (; ns < end; ++ns) {
		initialize_namespace(ns);
	}

	release(&namespace_table.lock);

	{
		struct namespace *ns = &namespace_table.namespaces[0];
		acquire(&ns->lock);
		ns->slot_state = TAKEN;
		release(&ns->lock);
	}
	return 0;
}

// Must be called after namespaceinit. Returns a pointer to the global
// namespace, currently the first namespace in the namespace in the namespace
// table.
struct namespace *global_ns(void) {
	return namespace_table.namespaces;
}

// Removes the ptr from ns. Assumes ns is valid. Returns err if ns is AVAILABLE
// (a state error) or the ptr is not in ns.
int remove_from_ns(struct namespace *ns, void *ptr) {

	acquire(&ns->lock);

	if (ns->slot_state == AVAILABLE) {
		release(&ns->lock);
		return -1;
	}


	struct ns_object *ns_obj = ns->namespaced_objects;
	struct ns_object *end = &ns->namespaced_objects[N_NS_OBJ];
	for (; ns_obj < end; ++ns_obj) {
		if (ns_obj->pointer == ptr) {
			ns_obj->kind = NONE;
			release(&ns->lock);
			return 0;
		}
	}

	release(&ns->lock);
	return -1;
}

// Attach a process to the namespace. Assumes ns is valid (e.g., from global_ns
// or create_ns) and proc is valid;
int attach_proc_to_ns(struct namespace *ns, struct proc *proc) {

	acquire(&ns->lock);

	if (ns->slot_state == AVAILABLE) {
		release(&ns->lock);
		return -1;
	}

	struct ns_object *ns_obj = ns->namespaced_objects;
	struct ns_object *end = &ns->namespaced_objects[N_NS_OBJ];

	for (; ns_obj < end; ++ns_obj) {
		if (ns_obj->kind == NONE) {
			ns_obj->pointer = proc;
			ns_obj->kind = PROCESS;
			proc->ns = ns;
			release(&ns->lock);
			return 0;
		}
	}

	// Did not find an available slot
	release(&ns->lock);
	return -1;
}

// Destroy the namespace ns. Assumes ns is valid. If ns has objects, will return
// error and make no changes to the ns.
int destroy_ns(struct namespace *ns) {

	acquire(&ns->lock);
	struct ns_object *ns_object = ns->namespaced_objects;
	struct ns_object *end = &ns->namespaced_objects[N_NS];

	// Cleanup only if there are no objects attached to this NS
	for (; ns_object < end; ++ns_object) {
		if (ns_object->kind != NONE) {
			release(&ns->lock);
			return -1;
		}
	}
	ns->slot_state = AVAILABLE;
	release(&ns->lock);
	return 0;
}
