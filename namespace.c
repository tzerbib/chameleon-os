#include "types.h"
#include "defs.h"
#include "param.h"
#include "namespace.h"
#include "mmu.h"          // so that proc.h gets its requisite types and defs
#include "proc.h"
#include "extensions.h"

struct {
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
	}

	// alocate maps
	ns->maps = hm_alloc(); 
	if (ns->maps == 0){
		acquire(&ns->lock); 
		ns->slot_state = AVAILABLE; 
		release(&ns->lock); 
		return (void*)0x0; 
	}

	return ns;
}

// Gets the ID of the namespace. Assumes ns is a valid namespace.
int get_nsid(struct namespace *ns) {
	struct namespace *start = namespace_table.namespaces;
	return ns - start;
}

// Given an ID, gets a pointer to the namespace. This returns (void *)0x0 if id
// is invalid since ID can come from a systemcall. ID is invalid if the
// namespace is available or id < 0 or id > N_NS.
struct namespace *get_ns(int id) {
	if (id < 0 || id > N_NS) {
		return (void *)0x0;
	}

	struct namespace *ns = &namespace_table.namespaces[id];
	acquire(&ns->lock);
	int slot_state = ns->slot_state;
	release(&ns->lock);

	if (slot_state == AVAILABLE) {
		return (void *)0x0;
	} 
	return &namespace_table.namespaces[id];
}

void initialize_namespace(struct namespace *ns) {
	acquire(&ns->lock);
	ns->slot_state = AVAILABLE;
	ns->maps = 0; 
	struct ns_object* ns_obj = ns->namespaced_objects;
	struct ns_object* ns_obj_end = &ns->namespaced_objects[N_NS_OBJ];
	for (; ns_obj < ns_obj_end; ++ns_obj) {
		ns_obj->kind = NONE;
	}
	struct ns_object *ns_ext = ns->namespaced_exts;
	struct ns_object *ns_ext_end = &ns->namespaced_exts[N_NS_EXT];
	for (; ns_ext < ns_ext_end; ++ns_ext) {
		ns_ext->kind = NONE;
	}
	release(&ns->lock);
}

// Must be called on OS init. initializes the namespace table
int namespaceinit(void) {

	// First initialize every namespace in the table
	struct namespace *ns = namespace_table.namespaces;
	struct namespace *end = &namespace_table.namespaces[N_NS];
	for (; ns < end; ++ns) {
		initialize_namespace(ns);
	}

	// Then mark the global namespace at 0
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
// table. See namespaceinit for initialization.
struct namespace *global_ns(void) {
	return namespace_table.namespaces;
}

// Removes the ptr from ns. Assumes ns is valid. Returns err if ns is AVAILABLE
// (a state error) or the ptr is not in ns. If error, no changes are made.
int remove_ptr_from_ns(struct namespace *ns, void *ptr, enum ns_entry_kind kind) {
	acquire(&ns->lock);

	if (ns->slot_state == AVAILABLE) {
		release(&ns->lock);
		return -1;
	}

	struct ns_object *ns_obj = ns->namespaced_objects;
	struct ns_object *end = &ns->namespaced_objects[N_NS_OBJ];
	
	if (kind == EXTENSION) {
		ns_obj = ns->namespaced_exts;
	  end = &ns->namespaced_exts[N_NS_EXT];
	}

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

int remove_proc_from_ns(struct namespace* ns, struct proc* proc) {
	return remove_ptr_from_ns(ns, (void*)proc, PROCESS);
}

int remove_ext_from_ns(struct namespace* ns, struct extension* ext) {
	return remove_ptr_from_ns(ns, (void*)ext, EXTENSION);
}

// Attach pointer to ns. Assumes ns is valid.
// Caller of this function is responsible for changing states of what ptr is pointer to.
int attach_ptr_to_ns(struct namespace *ns, void* ptr, enum ns_entry_kind kind) {
	acquire(&ns->lock);

	if (ns->slot_state == AVAILABLE) {
		goto bad;
	}

	struct ns_object* ns_obj = 0x0;
	struct ns_object* end = 0x0;
	if (kind != EXTENSION) {
		ns_obj = ns->namespaced_objects;
		end = &ns->namespaced_objects[N_NS_OBJ];
	} else {
		ns_obj = ns->namespaced_exts;
		end = &ns->namespaced_exts[N_NS_EXT];
	}

	for (; ns_obj < end; ++ns_obj) {
		if (ns_obj->kind == NONE) {
			ns_obj->pointer = ptr;
			ns_obj->kind = kind;
			release(&ns->lock);
			return 0;
		}
	}

	bad:
	release(&ns->lock);
	return -1;
}

// Attach a process to the namespace. Assumes ns is valid (e.g., from global_ns
// or create_ns) and proc is valid. If successful, proc->ns is changed and proc
// is added to ns. On failure, nothing is changed.
int attach_proc_to_ns(struct namespace *ns, struct proc *proc) {
	if (attach_ptr_to_ns(ns, (void*)proc, PROCESS) == -1) {
		return -1;
	}

	proc->ns = ns;
	return 0;
}

// Attach extension to namespace. Calls attach_ptr_to_ns helper and updates 
int attach_ext_to_ns(struct namespace* ns, struct extension* ext) {
	if (ext->state == EXT_UNUSED) {
		return -1;
	}

	if (attach_ptr_to_ns(ns, (void*)ext, EXTENSION) == -1) {
		return -1;
	}

	ext->ns = ns;

	return 0;
}


// Destroy the namespace ns. Assumes ns is valid. If ns has objects, will return
// error and make no changes to the ns.
int destroy_ns(struct namespace *ns) {
	acquire(&ns->lock);
	struct ns_object *ns_object = ns->namespaced_objects;
	struct ns_object *end = &ns->namespaced_objects[N_NS_OBJ];

	// Cleanup only if there are no objects attached to this NS
	for (; ns_object < end; ++ns_object) {
		if (ns_object->kind != NONE) {
			cprintf("Failed to destroy ns 0x%p\n", ns);
			release(&ns->lock);
			return -1;
		}
	}
	ns->slot_state = AVAILABLE;
	release(&ns->lock);
	
	// free maps (done after releasing ns->lock to prevent lock-ordering)
	if (ns->maps != 0){
		hm_free(ns->maps);
		ns->maps = 0;
	}


	return 0;
}
